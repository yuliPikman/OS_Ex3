#include "Barrier.h"
#include <stdexcept>
#include <iostream>
#include <thread>

#define DEBUG_PRINT(x) std::cout << x << std::endl;


Barrier::Barrier(int numThreads)
        : numThreads(numThreads), count(0), generation(0) {
    if (numThreads <= 0) {
        throw std::invalid_argument("Barrier must be initialized with positive number of threads.");
    }
}

void Barrier::barrier() {
    std::unique_lock<std::mutex> lock(mutex);
    int gen = generation;

    DEBUG_PRINT("Thread reached barrier (generation " << gen << ")")

    if (++count == numThreads) {
        generation++;
        count = 0;
        DEBUG_PRINT("Barrier released generation " << gen)
        cv.notify_all();
    } else {
        cv.wait(lock, [this, gen] { return gen != generation; });
        DEBUG_PRINT("Thread passed barrier generation " << gen)
    }
}
