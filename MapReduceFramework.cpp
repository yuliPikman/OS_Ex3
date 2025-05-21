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

#define DEBUG_PRINT(x) std::cout << x << '\n';
;


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
    std::atomic<uint64_t> atomicJobState;
    std::atomic<uint32_t> reduceGroupsDone;

    std::vector<IntermediateVec> shuffledVectorsQueue;
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
          atomicJobState(packState(UNDEFINED_STAGE, 0, 0)),
          reduceGroupsDone(0),
          joined(false) {}
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
    
    state->percentage = (total == 0) ? 0.0f : (float)processed / total * 100.0f;

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
    // מניעת חריגה – כדי למנוע percentage > 100
    if (total > 0 && processed > total) {
        processed = total;
    }

    uint64_t newPacked = packState(stage, processed, total);
    uint64_t current = jobContext->atomicJobState.load();

    while (true) {
        stage_t currStage = static_cast<stage_t>(current >> 62);
        uint32_t currProcessed = (current >> 31) & 0x7FFFFFFF;

        // רק אם השלב החדש גבוה יותר או באותו שלב אך יותר התקדם
        if (stage < currStage || (stage == currStage && processed <= currProcessed)) {
            return; // אל תעדכן — מדובר בנסיגה
        }

        if (jobContext->atomicJobState.compare_exchange_weak(current, newPacked)) {
            break; // הצלחנו לעדכן
        }
        // אחרת current קיבל את הערך העדכני וננסה שוב
    }
}




JobHandle startMapReduceJob(const MapReduceClient& client,
                            const InputVec& inputVec, OutputVec& outputVec,
                            int multiThreadLevel) {
    if (multiThreadLevel <= 0 || multiThreadLevel >= 20000000) {
        std::cout << "system error: invalid number of threads requested\n";
        std::exit(1);
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
            jobContext->threadsVec.emplace_back(workerFunction, &jobContext->threadContextsVec[i]);
        }
    } catch (const std::system_error& e) {
        std::cout << "system error: " << e.what() << '\n';
        std::exit(1);
    }


    return (JobHandle) jobContext.release();
}


void workerFunction(ThreadContext* threadContext) {
    try {

        handleMapPhase(threadContext);
        handleSortPhase(threadContext);

        handleShufflePhase(threadContext);
        threadContext->jobContext->sortBarrier->barrier();

        handleReducePhase(threadContext);

    }
    catch (const std::exception& e) {
    }
    catch (...) {
    }
}




void handleMapPhase(ThreadContext* threadContext) {
    JobContext* jobContext = threadContext->jobContext;

    if (threadContext->threadID == 0) {
        jobContext->mapAtomicIndex = 0;
        jobContext->intermediatePairsCounter = 0;
        jobContext->shuffledPairsCounter = 0;
        updateJobState(jobContext, MAP_STAGE, 0, (uint32_t)jobContext->inputVec.size());
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

}



void handleSortPhase(ThreadContext* threadContext) {
    std::sort(threadContext->intermediateResults.begin(), threadContext->intermediateResults.end(),
              [](const IntermediatePair& a, const IntermediatePair& b) {
                  return *(a.first) < *(b.first);
              });

    threadContext->jobContext->sortBarrier->barrier();
}


void handleShufflePhase(ThreadContext* threadContext) {
    JobContext* jobContext = threadContext->jobContext;
    if (threadContext->threadID != 0) return;

    updateJobState(jobContext, SHUFFLE_STAGE, 0, 1);

    std::vector<IntermediatePair> allPairs;
    for (auto& ctx : jobContext->threadContextsVec) {
        allPairs.insert(allPairs.end(), ctx.intermediateResults.begin(), ctx.intermediateResults.end());
    }

    std::sort(allPairs.begin(), allPairs.end(),
              [](const IntermediatePair& a, const IntermediatePair& b) {
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

}




void handleReducePhase(ThreadContext* threadContext) {
    JobContext* jobContext = threadContext->jobContext;

    // שלב ראשון: כל ה־threads מחכים לסיום שלב SORT
    threadContext->jobContext->sortBarrier->barrier();

    // שלב שני: thread 0 מאפס משתנים
    if (threadContext->threadID == 0) {
        jobContext->reduceIndex = 0;
        jobContext->reduceGroupsDone = 0;  // ✅ אפס את המונה
    }

    // שלב שלישי: כל ה־threads מוודאים שהאיפוס הסתיים
    threadContext->jobContext->sortBarrier->barrier();

    uint32_t total = (uint32_t)jobContext->shuffledVectorsQueue.size();
    int i = jobContext->reduceIndex.fetch_add(1);

    while (i < (int)total) {
        jobContext->mapReduceClientRef.reduce(&jobContext->shuffledVectorsQueue[i], threadContext);

        uint32_t done = jobContext->reduceGroupsDone.fetch_add(1) + 1;  // ✅ המשתמש הנכון
        updateJobState(jobContext, REDUCE_STAGE, done, total);


        i = jobContext->reduceIndex.fetch_add(1);
    }

    // סיום — כל ה־threads מחכים
    threadContext->jobContext->sortBarrier->barrier();
}




void closeJobHandle(JobHandle job) {
    if (job == nullptr) {
        throw std::runtime_error("closeJobHandle called on null JobHandle");
    }
    waitForJob(job);
    auto* jobContext = static_cast<JobContext*>(job);
    std::unique_ptr<JobContext> cleanup(jobContext);
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

