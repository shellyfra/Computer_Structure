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

void CORE_BlockedMT() {
   //int threads = SIM_GetThreadsNum();
   // c'tor for threads
   // while (not all halt)
   //   do queue of threads number ( enqueue dequeue)
   //

}

void CORE_FinegrainedMT() {
}

double CORE_BlockedMT_CPI(){
	return 0;
}

double CORE_FinegrainedMT_CPI(){
	return 0;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
}
