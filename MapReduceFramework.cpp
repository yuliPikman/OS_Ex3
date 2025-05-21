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

uint64_t packState(stage_t stage, uint32_t processed, uint32_t total) {
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

void getJobState(JobHandle job, JobState* state) {
    if (job == nullptr || state == nullptr) {
        throw std::runtime_error("Invalid argument to getJobState");
    }

    auto* jobContext = static_cast<JobContext*>(job);
    uint64_t raw = jobContext->atomicJobState.load();

    stage_t stage = static_cast<stage_t>(raw >> 62);
    uint32_t processed = (raw >> 31) & 0x7FFFFFFF;
    uint32_t total = raw & 0x7FFFFFFF;

    state->stage = stage;
    state->percentage = (total == 0) ? 0.0f : (float)processed / total;
}


void emit2(K2* key, V2* value, void* context) {
    if (context == nullptr || key == nullptr || value == nullptr) {
        throw std::runtime_error("emit2 received nullptr argument");
    }
    auto* threadContext = static_cast<ThreadContext*>(context);
    if (threadContext->intermediateResults.size() > 1000000) {
        throw std::runtime_error("Too many intermediate results");
    }
    threadContext->intermediateResults.emplace_back(key, value);
    threadContext->jobContext->intermediatePairsCounter++;
}


void emit3(K3* key, V3* value, void* context) {
    if (context == nullptr || key == nullptr || value == nullptr) {
        throw std::runtime_error("emit3 received nullptr argument");
    }
    auto* threadContext = static_cast<ThreadContext*>(context);
    JobContext* jobContext = threadContext->jobContext;
    std::lock_guard<std::mutex> lock(jobContext->writeToOutputVecMutex);
    jobContext->outputVec.emplace_back(key, value);
    jobContext->shuffledPairsCounter++;
}


void updateJobState(JobContext* jobContext, stage_t stage, uint32_t processed, uint32_t total) {
    uint64_t packed = packState(stage, processed, total);
    jobContext->atomicJobState.store(packed);

    DEBUG_PRINT("[updateJobState] Stage: " << stage << ", Progress: " << processed << "/" << total
                 << " (" << (total == 0 ? 0.0 : 100.0 * (double)processed / total) << "%)")
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

void workerFunction(ThreadContext* threadContext) {
    try {
        JobContext* jobContext = threadContext->jobContext;

        DEBUG_PRINT("Thread " << threadContext->threadID << " started.")

        if (threadContext->threadID == 0) {
            jobContext->mapAtomicIndex = 0;
            jobContext->intermediatePairsCounter = 0;
            jobContext->shuffledPairsCounter = 0;
            updateJobState(jobContext, MAP_STAGE, 0, (uint32_t)jobContext->inputVec.size());
            DEBUG_PRINT("MAP stage started")
        }

        size_t index = jobContext->mapAtomicIndex.fetch_add(1);
        while (index < jobContext->inputVec.size()) {
            auto& pair = jobContext->inputVec[index];
            jobContext->mapReduceClientRef.map(pair.first, pair.second, threadContext);
            index = jobContext->mapAtomicIndex.fetch_add(1);

            uint32_t done = jobContext->intermediatePairsCounter.load();
            uint32_t total = (uint32_t)jobContext->inputVec.size();
            updateJobState(jobContext, MAP_STAGE, done, total);
        }

        DEBUG_PRINT("Thread " << threadContext->threadID << " finished MAP phase.")

        std::sort(threadContext->intermediateResults.begin(), threadContext->intermediateResults.end(),
                  [](const IntermediatePair& a, const IntermediatePair& b) {
                      return *(a.first) < *(b.first);
                  });

        DEBUG_PRINT("Thread " << threadContext->threadID << " finished SORT phase.")

        jobContext->sortBarrier->barrier();

        if (threadContext->threadID == 0) {
            DEBUG_PRINT("SHUFFLE stage started")
            updateJobState(jobContext, SHUFFLE_STAGE, 0, 1);
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
                processed += group.size();
                updateJobState(jobContext, SHUFFLE_STAGE, (uint32_t)processed, (uint32_t)total);
            }
            DEBUG_PRINT("SHUFFLE stage completed")
            DEBUG_PRINT("Total reduce groups: " << jobContext->shuffledVectorsQueue.size());

        }

        jobContext->sortBarrier->barrier();

        jobContext->sortBarrier->barrier();

        if (threadContext->threadID == 0) {
            jobContext->reduceIndex = 0;
            DEBUG_PRINT("REDUCE stage started")
        }

        jobContext->sortBarrier->barrier();

        int i = jobContext->reduceIndex.fetch_add(1);
        uint32_t total = (uint32_t)jobContext->shuffledVectorsQueue.size();
        while (i < (int)total) {
            jobContext->mapReduceClientRef.reduce(&jobContext->shuffledVectorsQueue[i], threadContext);

            // מגדיל רק אם באמת בוצע emit3 (וזה נכון לפרויקטים שמפיקים זוג אחד לקבוצה)
            jobContext->shuffledPairsCounter++;  // אופציונלי אם אתה סופר ידנית

            uint32_t done = (uint32_t)jobContext->shuffledPairsCounter.load();
            updateJobState(jobContext, REDUCE_STAGE, done, total);

            DEBUG_PRINT("Thread " << threadContext->threadID << " reduced group " << i)

            i = jobContext->reduceIndex.fetch_add(1);
        }

        DEBUG_PRINT("Thread " << threadContext->threadID << " finished REDUCE phase.")

    }
    catch (const std::exception& e) {
        DEBUG_PRINT("Thread " << threadContext->threadID << " encountered exception: " << e.what());
    }
    catch (...) {
        DEBUG_PRINT("Thread " << threadContext->threadID << " encountered unknown exception.");
    }
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

void waitForJob(JobHandle job) {
    auto* jobContext = static_cast<JobContext*>(job);
    if (!jobContext->joined) {
        for (auto &t : jobContext->threadsVec) {
            if (t.joinable()) {
                t.join();
            }
        }
        jobContext->joined = true;
    }
}

