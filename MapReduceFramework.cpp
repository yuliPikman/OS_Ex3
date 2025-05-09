#include "Barrier.h"
#include "MapReduceFramework.h"


struct JobState {
    stage_t stage;
    float percentage;
};

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
    std::mutex stateMutex; //for JobState
    std::mutex shuffleMutex; // In reduce after shuffle
    Barrier* sortBarrier; //for sort

    JobContext(const MapReduceClient& client,
               const InputVec& inputVec,
               OutputVec& outputVec,
               int multiThreadLevel)
            : mapReduceClientRef(client),
              inputVec(inputVec),
              outputVec(outputVec),
              multiThreadLevel(multiThreadLevel),
              sortBarrier(nullptr)
    {
        state.stage = UNDEFINED;
        state.percentage = 0.0;
    }
};

JobHandle startMapReduceJob(const MapReduceClient& client,
                            const InputVec& inputVec, OutputVec& outputVec,
                            int multiThreadLevel) {
    JobContext* jobContext = new JobContext(client, inputVec, outputVec, multiThreadLevel);
    jobContext->sortBarrier = new Barrier(multiThreadLevel); //TODO: remember to delete

    for (int i = 0; i < multiThreadLevel; ++i) {
        jobContext->threadContextsVec.emplace_back(i, IntermediateVec(), jobContext);
        jobContext->threadsVec.emplace_back(workerFunction, &jobContext->threadContextsVec[i]);

    }




    return (JobHandle) jobContext;
}

