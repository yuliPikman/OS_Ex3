#ifndef OS_EX3_JOBCONTEXT_H
#define OS_EX3_JOBCONTEXT_H
#include "string"
#include "MapReduceFramework.h"

struct ThreadContext;
struct JobContext;


// Constant variables
static const std::string ERROR_MSG_BARRIER = "Error: Barrier must be initialized with positive number of threads.";
static const int ERROR_EXIT_CODE = 1;
static const std::string INVALID_NUM_OF_THREADS_ERROR = "system error: invalid number of threads requested\n";
static const std::string INVALID_ARGUMENT_FOR_JOB_STATE_ERROR = "Invalid argument to getJobState";
static const std::string EMIT2_NULL_POINTER_ERROR = "emit2 received nullptr argument";
static const std::string EMIT3_NULL_POINTER_ERROR = "emit3 received nullptr argument";
static const std::string TOO_MANY_INTERMEDIATE_RESULTS_ERROR = "Too many intermediate results";
static const std::string INPUT_VECTOR_EMPTY_ERROR = "Input vector is empty";
static const std::string NULL_JOB_HANDLE_ERROR = "closeJobHandle called on null JobHandle";


/**
 * @brief Packs the job's current state into a single 64-bit integer.
 *
 * Encodes the current stage, number of processed elements, and total elements
 * into a single `uint64_t` value using bit-shifting. This packed representation
 * is used to atomically store and update job progress in a thread-safe way.
 *
 * Layout (from most to least significant bits):
 * - Bits 63–62: stage_t (2 bits)
 * - Bits 61–31: processed count (31 bits)
 * - Bits 30–0 : total count (31 bits)
 *
 * @param stage The current stage of the job (MAP_STAGE, SHUFFLE_STAGE, etc.).
 * @param processed Number of elements processed so far.
 * @param total Total number of elements to process.
 * @return A 64-bit integer encoding the full job state.
 */
uint64_t packState(stage_t stage, uint32_t processed, uint32_t total);


/**
* @brief This function is the main function executed by each worker thread.
*
* Executes all phases of the MapReduce pipeline: Map, Sort, Shuffle (by thread 0 only),
* and Reduce. Each thread runs this function independently.
*
* @param threadContext Pointer to the calling thread's context.
*/
void workerFunction(ThreadContext* threadContext);


/**
 * @brief This function executes the Map phase for the given thread.
 *
 * Processes input pairs using the user-defined map function and stores the
 * intermediate results locally within the thread context.
 *
 * @param threadContext Pointer to the thread's context.
 */
void handleMapPhase(ThreadContext* threadContext);

/**
 * @brief This function sorts the intermediate key-value pairs for a single thread.
 *
 * Each thread sorts its own intermediate results by key. A barrier ensures that
 * all threads complete this phase before proceeding.
 *
 * @param threadContext Pointer to the thread's context.
 */
void handleSortPhase(ThreadContext* threadContext);

/**
 * @brief This function performs the Shuffle phase — executed by thread 0 only.
 *
 * Aggregates all intermediate results from every thread, sorts them globally by key,
 * and groups them into key-specific vectors. These groups are stored in the shared
 * shuffledVectorsQueue for the Reduce phase.
 *
 * @param threadContext Pointer to the thread's context (must be thread ID 0).
 */
void handleShufflePhase(ThreadContext* threadContext);


/**
 * @brief This function executes the Reduce phase for the given thread.
 *
 * Each thread pulls a group of key-matching pairs from the shuffled queue and applies
 * the user-defined reduce function. A barrier ensures that the Reduce phase
 * starts only after all threads have completed sorting and shuffling.
 *
 * @param threadContext Pointer to the thread's context.
 */
void handleReducePhase(ThreadContext* threadContext);


/**
 * @brief This function sorts by groups intermediate key-value pairs by key and stores them for the Reduce phase.
 *
 * This function assumes that allPairs is globally sorted by key (`K2*`). It iterates over the pairs,
 * groups adjacent elements with the same key, and stores each group in the `shuffledVectorsQueue`
 * inside the provided JobContext. Each group will later be processed by the Reduce phase.
 *
 * @param allPairs A sorted vector of intermediate (K2*, V2*) pairs from all threads.
 * @param jobContext A pointer to the JobContext where the grouped data will be stored.
 */
void groupByKeyAndStore(const std::vector<IntermediatePair>& allPairs, JobContext* jobContext);

/**
 * @brief This function performs the Reduce phase by processing grouped intermediate pairs.
 *
 * Each thread independently fetches a group from the shared `shuffledVectorsQueue` (based on an atomic index),
 * and applies the user-defined `reduce` function to that group. The results are stored using `emit3`.
 *
 * This function is called after synchronization to ensure all threads have completed shuffling.
 *
 * @param jobContext A pointer to the global JobContext.
 * @param threadContext A pointer to the calling thread's context, used for bookkeeping and accessing state.
 */
void reduceGroups(JobContext* jobContext, ThreadContext* threadContext);


/**
 * @brief Validates the input before starting the MapReduce job.
 *
 * This function performs preliminary checks before starting the Map phase.
 * It ensures that the number of threads is within a valid range and that
 * the input vector is not empty.
 *
 * - If the thread count is invalid, it prints an error message and exits the program.
 * - If the input vector is empty, it throws a runtime error.
 *
 * @param multiThreadLevel The number of threads requested for the job.
 * @param inputVec The input vector of (K1*, V1*) pairs to be processed.
 *
 * @throws std::runtime_error If the input vector is empty.
 * @exit Terminates the program if thread count is invalid.
 */
void start_map_check(int multiThreadLevel, const InputVec& inputVec);



#endif //OS_EX3_JOBCONTEXT_H
