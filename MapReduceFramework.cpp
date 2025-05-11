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
    //Map phase
    JobContext* jobContext = threadContext->jobContext;
    int index = jobContext->mapAtomicIndex.fetch_add(1);
    while (index < jobContext->inputVec.size()) {
        auto& pair = jobContext->inputVec[index];
        jobContext->mapReduceClientRef.map(pair.first, pair.second, threadContext);
        index = jobContext->mapAtomicIndex.fetch_add(1);
    }

    //Sort phase - by key
    std::sort(
            threadContext->intermediateResults.begin(),
            threadContext->intermediateResults.end(),
            [](const IntermediatePair& result1, const IntermediatePair& result2) {
                return *(result1.first) < *(result2.first);
            }
    );
}

void waitForJob(JobHandle job) {
    auto* jobContext = static_cast<JobContext*>(job);
    if (!jobContext->joined) {
        for (auto &t: jobContext->threadsVec) {
            t.join();
        }
    }
    jobContext->joined = true;
}

void closeJobHandle(JobHandle job) {
    waitForJob(job);
    auto* jobContext = static_cast<JobContext*>(job);
    delete jobContext->sortBarrier;
    delete jobContext;
}