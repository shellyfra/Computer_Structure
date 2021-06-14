/* 046267 Computer Architecture - Spring 2021 - HW #4 */

#include "core_api.h"
#include "sim_api.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>

enum status{HALT = 0, WAITING, READY};
class Sched {
public:
    status stat_thread;
    int countdown_thread;
    int cur_line;
    uint32_t thread_id;
    int reg[REGS_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0}; // init all regs to 0
    explicit Sched(int tid): stat_thread(READY), countdown_thread(0), cur_line(0), thread_id(tid){
    }

};
class ThreadsStatus {
public:
    std::vector<Sched*> map_thread;
    int num_threads;
    int load_cycles;
    int store_cycles;
    int context_switch_cost;
    int total_cycles;
    int num_instructions;

    ThreadsStatus(int num_of_threads, int load_cyc, int store_cyc, int switch_overhead = 0):
    num_threads(num_of_threads), load_cycles(load_cyc), store_cycles(store_cyc), context_switch_cost(switch_overhead),
    total_cycles(0), num_instructions(0){
        map_thread.resize(num_of_threads);
        for (int i = 0; i <num_of_threads ; ++i) {
            map_thread[i] = new Sched(i);
        }
    }
    ~ThreadsStatus() {
        for (int i = 0; i <num_threads ; ++i) {
            delete map_thread[i];
        }
    }
    ThreadsStatus(ThreadsStatus& other) = default;
    bool readyThreads() {
        for (int i = 0; i <num_threads ; ++i) {
           if(map_thread[i]->stat_thread == READY){
               return true;
           }
        }
        return false;
    }
};
ThreadsStatus* blocked_multithread;
/*
 *This function contains a full simulation of a blocked MT machine.
 * The function reaches it's end when all of the threads reach status 'HALT'.
 */
void CORE_BlockedMT() {
   int threads_num = SIM_GetThreadsNum();
   if (threads_num <= 0){
       return;
   }
    blocked_multithread = new ThreadsStatus(threads_num, SIM_GetLoadLat(), SIM_GetStoreLat(), SIM_GetSwitchCycles());

   std::queue<int> threads_queue;
    for (int i = 0; i < threads_num ; ++i) {
        threads_queue.push(i); // TODO check that its FIFO queue!
    }

    while(!threads_queue.empty()){
        int running_thread = threads_queue.front();
        Instruction new_inst;
        while (blocked_multithread->map_thread[running_thread]->stat_thread == READY) { // while thread can still run
            SIM_MemInstRead(blocked_multithread->map_thread[running_thread]->thread_id, &new_inst, running_thread);
            switch (new_inst.opcode) {
                case CMD_NOP:
                    break;
                case CMD_ADD:
                    break;
                case CMD_SUB:
                    break;
                case CMD_ADDI:
                    break;
                case CMD_SUBI:
                    break;
                case CMD_LOAD:
                    break;
                case CMD_STORE:
                    break;
                case CMD_HALT:
                    blocked_multithread->map_thread[running_thread]->stat_thread = HALT;
                    blocked_multithread->total_cycles++; // 1 for HALT
                    break;
                default:
                    break;
            }
        }
        if (blocked_multithread->map_thread[running_thread]->stat_thread == HALT){
            threads_queue.pop(); // we are not removing elements from queue unless its halted ( in order to try working on the same thread that we are on it now)

        }
        // check if this thread can't run but others can
        if (blocked_multithread->readyThreads() && (blocked_multithread->map_thread[running_thread]->stat_thread != READY)) {
            blocked_multithread->total_cycles += blocked_multithread->context_switch_cost;
            threads_queue.pop();
            threads_queue.push(running_thread);
        }
        // check if switch overhead is needed
        // if this thread can still run -> run it
        // update all threads countdown_thread with -1
        //
    }
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
    delete blocked_multithread;
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