#include "Barrier.h"
#include "MapReduceFramework.h"
#include <atomic>
#include <condition_variable>
#include <vector>


struct ThreadContext {
    int threadID;
    IntermediateVec intermediateResults;
    JobContext* jobContext;

    ThreadContext(int threadID,
                  IntermediateVec intermediateResults,
                  JobContext* jobContext)
                  : threadID(threadID),
                    intermediateResults(intermediateResults),
                    jobContext(jobContext) {}

};

struct JobContext {
    const MapReduceClient& mapReduceClientRef;
    const InputVec& inputVec;
    OutputVec& outputVec;
    int multiThreadLevel;
    std::vector<std::thread> threadsVec;
    std::vector<ThreadContext> threadContextsVec;
    std::mutex writeToOutputVecMutex;
    JobState state;
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
        state.stage = UNDEFINED;
        state.percentage = 0.0;
    }
};

JobHandle startMapReduceJob(const MapReduceClient& client,
                            const InputVec& inputVec, OutputVec& outputVec,
                            int multiThreadLevel) {
    JobContext* jobContext = new JobContext(client, inputVec, outputVec, multiThreadLevel); //TODO: remember to delete
    jobContext->sortBarrier = new Barrier(multiThreadLevel); //TODO: remember to delete

    for (int i = 0; i < multiThreadLevel; ++i) {
        jobContext->threadContextsVec.emplace_back(i, IntermediateVec(), jobContext);
        //TODO: workerFunction is responsible for all the map-reduce process! we need to write it
        jobContext->threadsVec.emplace_back(workerFunction, &jobContext->threadContextsVec[i]);
    }

    return (JobHandle) jobContext;
}

void getJobState(JobHandle job, JobState* state) {
    JobContext* jobContext = static_cast<JobContext*>(job);
    std::lock_guard<std::mutex> lock(jobContext->stateMutex);
    jobContext->state = state;
    *state = jobContext->state;
}

void emit2 (K2* key, V2* value, void* context) {
    ThreadContext* threadContext = static_cast<ThreadContext*>(context);
    threadContext->intermediateResults.push_back({key, value});
    threadContext->jobContext->intermediatePairsCounter++;
}

void emit3 (K3* key, V3* value, void* context) {
    ThreadContext* threadContext = static_cast<ThreadContext*>(context);
    JobContext* jobContext = threadContext->jobContext;
    std::lock_guard<std::mutex> lock(jobContext->writeToOutputVecMutex);
    jobContext->outputVec.push_back({key, value});
    jobContext->shuffledPairsCounter++;
}
