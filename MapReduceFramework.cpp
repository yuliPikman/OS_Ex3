#include "Barrier.h"
#include "MapReduceFramework.h"
#include "Utils.h"
#include <atomic>
#include <condition_variable>
#include <utility>
#include <vector>
#include <algorithm>
#include <thread>
#include <stdexcept>
#include <iostream>
#include <memory>

#define DEBUG_PRINT(x) std::cout << x << std::endl;

struct ThreadContext;

uint64_t packState(Stage stage, uint32_t processed, uint32_t total) {
    return ((uint64_t)stage << 62) | ((uint64_t)processed << 31) | total;
}

struct JobContext {
    const MapReduceClient& mapReduceClientRef;
    const InputVec& inputVec;
    OutputVec& outputVec;
    int multiThreadLevel;
    std::vector<std::thread> threadsVec;
    std::vector<ThreadContext> threadContextsVec;
    std::mutex writeToOutputVecMutex;
    std::mutex shuffleMutex;
    std::unique_ptr<Barrier> sortBarrier;
    std::atomic<int> mapAtomicIndex;
    std::atomic<int> reduceIndex;
    std::atomic<size_t> shuffledPairsCounter;
    std::atomic<size_t> intermediatePairsCounter;
    std::vector<IntermediateVec> shuffledVectorsQueue;
    bool joined;
    std::atomic<uint64_t> atomicJobState;

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
              joined(false),
              atomicJobState(packState(UNDEFINED_STAGE, 0, 0)) {}
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




void updateJobState(JobContext* jobContext, Stage stage, uint32_t processed, uint32_t total) {
    jobContext->atomicJobState.store(packState(stage, processed, total));
}


JobHandle startMapReduceJob(const MapReduceClient& client,
                            const InputVec& inputVec, OutputVec& outputVec,
                            int multiThreadLevel) {
    if (multiThreadLevel <= 0) {
        throw std::runtime_error("multiThreadLevel must be >= 1");
    }
    if (inputVec.empty()) {
        throw std::runtime_error("Input vector is empty");
    }

    std::unique_ptr<JobContext> jobContext(new JobContext(client, inputVec, outputVec, multiThreadLevel));

    try {
        jobContext->sortBarrier = std::unique_ptr<Barrier>(new Barrier(multiThreadLevel));
        jobContext->threadContextsVec.reserve(multiThreadLevel);
        jobContext->threadsVec.reserve(multiThreadLevel);

        for (int i = 0; i < multiThreadLevel; ++i) {
            jobContext->threadContextsVec.emplace_back(i, IntermediateVec(), jobContext.get());
        }

        for (int i = 0; i < multiThreadLevel; ++i) {
            try {
                jobContext->threadsVec.emplace_back(workerFunction, &jobContext->threadContextsVec[i]);
            } catch (...) {
                for (auto& t : jobContext->threadsVec) {
                    if (t.joinable()) {
                        t.join();
                    }
                }
                throw std::runtime_error("Failed to create thread " + std::to_string(i));
            }
        }
    } catch (...) {
        throw;
    }

    return (JobHandle) jobContext.release();
}

void closeJobHandle(JobHandle job) {
    if (job == nullptr) {
        throw std::runtime_error("closeJobHandle called on null JobHandle");
    }
    waitForJob(job);
    auto* jobContext = static_cast<JobContext*>(job);
    std::unique_ptr<JobContext> cleanup(jobContext);
    // sortBarrier will be deleted automatically
}
