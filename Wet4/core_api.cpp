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
    tcontext regs_array{};

    explicit Sched(int tid) : stat_thread(READY), countdown_thread(0), cur_line(0), thread_id(tid) {
        for (int i = 0; i < REGS_COUNT; i++) {
            regs_array.reg[i] = 0;
        }
    }
    ~Sched() = default;
    Sched(Sched &other) = default;
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
ThreadsStatus* finegrained_multithread;
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
                    blocked_multithread->num_instructions++;
                    blocked_multithread->map_thread[running_thread]->stat_thread = HALT;
                    blocked_multithread->total_cycles++; // 1 for HALT
                    threads_queue.pop(); // TODO : not sure it needs to be here
                    break;
                default:
                    break;
            }
        }
        // check if this thread can't run but others can
        if (blocked_multithread->readyThreads()) {
            //&& (blocked_multithread->map_thread[running_thread]->stat_thread != READY) - DONT NEED BECAUSE WE ARE OUT OF THE WHILE LOOP
            blocked_multithread->total_cycles += blocked_multithread->context_switch_cost;
            threads_queue.pop();
            // we are not removing elements from queue unless its halted
            // ( in order to try working on the same thread that we are on it now)
            if (blocked_multithread->map_thread[running_thread]->stat_thread != HALT) {
                threads_queue.push(running_thread);
            }
        }
        else { // idle - no thread can run
            blocked_multithread->total_cycles++;
            for (int i = 0; i < blocked_multithread->num_threads; ++i) {
                if (blocked_multithread->map_thread[i]->stat_thread == WAITING){
                    blocked_multithread->map_thread[i]->countdown_thread--;
                    if (blocked_multithread->map_thread[i]->countdown_thread == 0){
                        blocked_multithread->map_thread[i]->stat_thread = READY;
                    }
                }
            }
        }
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
    double cpi = (double)blocked_multithread->total_cycles / (double)blocked_multithread->num_instructions;
    delete blocked_multithread;
    return cpi;
}

/**
 * calculated the performence of the system in CPI.
 * @return the performence fo the system in CPI units
 */
double CORE_FinegrainedMT_CPI(){
    double cpi = (double)finegrained_multithread->total_cycles / (double)finegrained_multithread->num_instructions;
    delete finegrained_multithread;
    return cpi;
}

/**
 *Function returns the register file of a given thread through a pointer named context
 * @param context
 * @param threadid - a given id of a thread
 */
void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
    *context = blocked_multithread->map_thread[threadid]->regs_array;
}

/**
 *Function returns the register file of a given thread through a pointer named context
 * @param context
 * @param threadid - a given id of a thread
 */
void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
    *context = finegrained_multithread->map_thread[threadid]->regs_array;
}