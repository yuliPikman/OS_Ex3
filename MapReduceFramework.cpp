#include "Barrier.h"
#include "MapReduceFramework.h"
#include "Utils.h"
#include <atomic>
#include <condition_variable>
#include <utility>
#include <vector>
#include <algorithm>
#include "thread"


struct ThreadContext;

struct JobContext {
    const MapReduceClient& mapReduceClientRef;
    const InputVec& inputVec;
    OutputVec& outputVec;
    int multiThreadLevel;
    std::vector<std::thread> threadsVec;
    std::vector<ThreadContext> threadContextsVec;
    std::mutex writeToOutputVecMutex;
    JobState state{};
    std::mutex stateMutex; //for JobState - if we have time we can do it more efficient
    std::mutex shuffleMutex; // In reduce after shuffle
    Barrier* sortBarrier; //for sort
    
    std::atomic<int> mapAtomicIndex; // for map phase distribution
    std::atomic<int> reduceIndex; // for reduce phase distribution
    std::atomic<size_t> shuffledPairsCounter; // for progress tracking in shuffle/reduce
    std::atomic<size_t> intermediatePairsCounter; // for progress tracking in shuffle/reduce
    std::vector<IntermediateVec> shuffledVectorsQueue; // the reduce queue
    bool joined;

    JobContext(const MapReduceClient& client,
               const InputVec& inputVec,
               OutputVec& outputVec,
               int multiThreadLevel)
            : mapReduceClientRef(client),
              inputVec(inputVec),
              outputVec(outputVec),
              multiThreadLevel(multiThreadLevel),
              sortBarrier(nullptr),
              mapAtomicIndex(0),
              reduceIndex(0),
              shuffledPairsCounter(0),
              intermediatePairsCounter(0),
              joined(false)
    {
        state.stage = UNDEFINED_STAGE;
        state.percentage = 0.0;
    }
};

struct ThreadContext {
    int threadID;
    IntermediateVec intermediateResults;
    JobContext* jobContext;

    ThreadContext(int threadID,
                  IntermediateVec intermediateResults,
                  JobContext* jobContext)
            : threadID(threadID),
              intermediateResults(std::move(intermediateResults)),
              jobContext(jobContext) {}

};


JobHandle startMapReduceJob(const MapReduceClient& client,
                            const InputVec& inputVec, OutputVec& outputVec,
                            int multiThreadLevel) {
    auto* jobContext = new JobContext(client, inputVec, outputVec, multiThreadLevel); //TODO: remember to delete
    jobContext->sortBarrier = new Barrier(multiThreadLevel); //TODO: remember to delete

    for (int i = 0; i < multiThreadLevel; ++i) {
        jobContext->threadContextsVec.emplace_back(i, IntermediateVec(), jobContext);
        //TODO: workerFunction is responsible for all the map-reduce process! we need to write it
        jobContext->threadsVec.emplace_back(workerFunction, &jobContext->threadContextsVec[i]);
    }

    return (JobHandle) jobContext;
}

void getJobState(JobHandle job, JobState* state) {
    auto* jobContext = static_cast<JobContext*>(job);
    std::lock_guard<std::mutex> lock(jobContext->stateMutex);
    *state = jobContext->state;
}

void emit2 (K2* key, V2* value, void* context) {
    auto* threadContext = static_cast<ThreadContext*>(context);
    threadContext->intermediateResults.emplace_back(key, value);
    threadContext->jobContext->intermediatePairsCounter++;
}

void emit3 (K3* key, V3* value, void* context) {
    auto* threadContext = static_cast<ThreadContext*>(context);
    JobContext* jobContext = threadContext->jobContext;
    std::lock_guard<std::mutex> lock(jobContext->writeToOutputVecMutex);
    jobContext->outputVec.emplace_back(key, value);
    jobContext->shuffledPairsCounter++;
}

//TODO: nir's part
void workerFunction(ThreadContext* threadContext) {
    JobContext* jobContext = threadContext->jobContext;

    // --------------------------- MAP PHASE ---------------------------
    if (threadContext->threadID == 0) {
        std::lock_guard<std::mutex> lock(jobContext->stateMutex);
        jobContext->state.stage = MAP_STAGE;
        jobContext->mapAtomicIndex = 0;
        jobContext->intermediatePairsCounter = 0;
        jobContext->shuffledPairsCounter = 0;
    }

    int index = jobContext->mapAtomicIndex.fetch_add(1);
    while (index < jobContext->inputVec.size()) {
        auto& pair = jobContext->inputVec[index];
        jobContext->mapReduceClientRef.map(pair.first, pair.second, threadContext);
        index = jobContext->mapAtomicIndex.fetch_add(1);

        // עדכון אחוזים בשלב MAP
        {
            std::lock_guard<std::mutex> lock(jobContext->stateMutex);
            jobContext->state.percentage = 
                (float)jobContext->intermediatePairsCounter.load() / jobContext->inputVec.size();
        }
    }

    // --------------------------- SORT PHASE ---------------------------
    std::sort(
        threadContext->intermediateResults.begin(),
        threadContext->intermediateResults.end(),
        [](const IntermediatePair& a, const IntermediatePair& b) {
            return *(a.first) < *(b.first);
        }
    );

    // ------------------------ BARRIER #1 (before shuffle) ------------------------
    jobContext->sortBarrier->barrier();

    // --------------------------- SHUFFLE PHASE (only thread 0) ---------------------------
    if (threadContext->threadID == 0) {
        {
            std::lock_guard<std::mutex> lock(jobContext->stateMutex);
            jobContext->state.stage = SHUFFLE_STAGE;
            jobContext->state.percentage = 0;
        }

        std::vector<IntermediatePair> allPairs;
        for (auto& ctx : jobContext->threadContextsVec) {
            allPairs.insert(allPairs.end(), ctx.intermediateResults.begin(), ctx.intermediateResults.end());
        }

        std::sort(allPairs.begin(), allPairs.end(), [](const IntermediatePair& a, const IntermediatePair& b) {
            return *(a.first) < *(b.first);
        });

        size_t total = allPairs.size();
        size_t processed = 0;
        for (size_t i = 0; i < total;) {
            K2* currentKey = allPairs[i].first;
            IntermediateVec group;
            while (i < total && !(*currentKey < *(allPairs[i].first)) && !(*(allPairs[i].first) < *currentKey)) {
                group.push_back(allPairs[i]);
                ++i;
            }
            jobContext->shuffledVectorsQueue.push_back(group);

            // עדכון אחוזים בשלב SHUFFLE
            processed += group.size();
            {
                std::lock_guard<std::mutex> lock(jobContext->stateMutex);
                jobContext->state.percentage = (float)processed / total;
            }
        }
    }

    // ------------------------ BARRIER #2 (before reduce) ------------------------
    jobContext->sortBarrier->barrier();

    // --------------------------- REDUCE PHASE ---------------------------
    if (jobContext->shuffledVectorsQueue.empty()) {
        return;
    }

    if (threadContext->threadID == 0) {
        jobContext->reduceIndex = 0;
        {
            std::lock_guard<std::mutex> lock(jobContext->stateMutex);
            jobContext->state.stage = REDUCE_STAGE;
            jobContext->state.percentage = 0;
        }
    }

    // optional barrier to ensure reduceIndex reset is visible
    jobContext->sortBarrier->barrier();

    int i = jobContext->reduceIndex.fetch_add(1);
    while (i < (int)jobContext->shuffledVectorsQueue.size()) {
        jobContext->mapReduceClientRef.reduce(&jobContext->shuffledVectorsQueue[i], threadContext);
        i = jobContext->reduceIndex.fetch_add(1);

        // עדכון אחוזים בשלב REDUCE
        {
            std::lock_guard<std::mutex> lock(jobContext->stateMutex);
            jobContext->state.percentage = 
                (float)jobContext->reduceIndex.load() / jobContext->shuffledVectorsQueue.size();
        }
    }
} 




void waitForJob(JobHandle job) {
    auto* jobContext = static_cast<JobContext*>(job);
    if (!jobContext->joined) {
        for (auto &t: jobContext->threadsVec) {
            t.join();
        }
        jobContext->joined = true;
    }
}

void closeJobHandle(JobHandle job) {
    waitForJob(job);
    auto* jobContext = static_cast<JobContext*>(job);
    delete jobContext->sortBarrier;
    delete jobContext;
}