#include "Barrier.h"
#include "Utils.h"
#include <stdexcept>

Barrier::Barrier(int numThreads)
        : numThreads(numThreads), count(0), generation(0) {
    if (numThreads <= 0) {
        throw std::invalid_argument(ERROR_MSG_BARRIER);
    }
}

void Barrier::barrier() {
    std::unique_lock<std::mutex> lock(mutex);
    int gen = generation;

    if (++count == numThreads) {
        generation++;
        count = 0;
        cv.notify_all();
    } else {
        cv.wait(lock, [this, gen] { return gen != generation; });
    }
}
