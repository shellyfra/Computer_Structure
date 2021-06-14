/* 046267 Computer Architecture - Spring 2021 - HW #4 */

#include "core_api.h"
#include "sim_api.h"

#include <stdio.h>
#include <vector>

enum status{HALT = 0, WAITING, READY};
class sched {
    status stat_thread;
    int countdown_thread;
};
class threads_status {
public:
    std::vector<sched> map_thread;
    int num_instructions;
    int total_cycles;
    int num_threads;
};

/*
 *This function contains a full simulation of a blocked MT machine.
 * The function reaches it's end when all of the threads reach status 'HALT'.
 */
void CORE_BlockedMT() {
   //int threads = SIM_GetThreadsNum();
   // c'tor for threads
   // while (not all halt)
   //   do queue of threads number ( enqueue dequeue)
   //

}

/*
 *This function contains a full simulation of a fine-grained MT machine.
 * The function reaches it's end when all of the threads reach status 'HALT'.
 */
void CORE_FinegrainedMT() {
}

/**
 * calculated the performence of the system in CPI.
 * @return the performence fo the system in CPI units
 */
double CORE_BlockedMT_CPI(){
    return 0;
}

/**
 * calculated the performence of the system in CPI.
 * @return the performence fo the system in CPI units
 */
double CORE_FinegrainedMT_CPI(){
    return 0;
}

/**
 *Function returns the register file of a given thread through a pointer named context
 * @param context
 * @param threadid - a given id of a thread
 */
void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
}

/**
 *Function returns the register file of a given thread through a pointer named context
 * @param context
 * @param threadid - a given id of a thread
 */
void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
}