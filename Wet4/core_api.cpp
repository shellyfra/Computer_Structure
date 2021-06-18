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
    //tcontext* regs_array;

    explicit Sched(int tid) : stat_thread(READY), countdown_thread(0), cur_line(0), thread_id(tid) {
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
    tcontext* regs_array;

    ThreadsStatus(int num_of_threads, int load_cyc, int store_cyc, int switch_overhead = 0):
        num_threads(num_of_threads), load_cycles(load_cyc), store_cycles(store_cyc), context_switch_cost(switch_overhead),
        total_cycles(0), num_instructions(0) {
            map_thread.resize(num_of_threads);
            for (int i = 0; i <num_of_threads ; i++) {
                map_thread[i] = new Sched(i);
            }
            regs_array = (tcontext*)malloc(num_of_threads * sizeof(tcontext));
            for(int k=0; k<num_of_threads; k++) {
                for (int i=0; i<REGS_COUNT; i++) {
                    regs_array[k].reg[i] = 0;
                }
            }
    }
    ~ThreadsStatus() {
        for (int i = 0; i <num_threads ; i++) {
            delete map_thread[i];
        }
        free(regs_array);
    }
    ThreadsStatus(ThreadsStatus& other) = default;
    bool readyThreads() {
        for (int i = 0; i <num_threads ; i++) {
           if(map_thread[i]->stat_thread == READY){
               return true;
           }
        }
        return false;
    }
    /* return the index of the next thread that's ready for run */
    int getNextReadyThread(int current_thread) { //todo: check if this function works as it should
        int index = 1;
        while (index++ <= num_threads) {
            if(map_thread[(current_thread+index)%num_threads]->stat_thread == READY){ //todo: check the formula
                return index%num_threads;
            }
        }
        return -1;
    }
};

ThreadsStatus* blocked_multithread;
ThreadsStatus* finegrained_multithread;

void addOrSubOperation (ThreadsStatus* multithread, int running_thread, Instruction* inst, bool is_add_inst) {
    int src1_idx = inst->src1_index;
    int src2_idx = inst->src2_index_imm;
    int dst = inst->dst_index;
    int src1 = multithread->regs_array[running_thread].reg[src1_idx];
    //if the command is REGISTER + REGISTER
    if (!inst->isSrc2Imm) {
        int src2 = multithread->regs_array[running_thread].reg[src2_idx];
        if (is_add_inst) {
            // dst <- src1 + src2
            multithread->regs_array[running_thread].reg[dst] = src1 + src2;
            //printf("ADD reg%d = %d\n", dst,src1 + src2);
        } else {
            // dst <- src1 - src2
            multithread->regs_array[running_thread].reg[dst] = src1 - src2;
            //printf("SUB reg%d = %d\n", dst,src1 - src2);
        }
    }
    //if the command is REGISTER + IMMEDIATE
    else {
        int src2 = src2_idx; // because it's an immediate
        if (is_add_inst) {
            // dst <- src1 + src2
            multithread->regs_array[running_thread].reg[dst] = src1 + src2;
            //printf("ADDI reg%d = %d\n", dst,src1 + src2);
        } else {
            // dst <- src1 - src2
            multithread->regs_array[running_thread].reg[dst] = src1 - src2;
            //printf("SUBI reg%d = %d\n", dst,src1 - src2);
        }
    }
}

void loadOrStoreOperation(ThreadsStatus* multithread, int running_thread, Instruction* inst, bool is_load) {
    //todo: do we need to increase the number of cycles here?
    //todo: or maybe set the countdown?
    int src1 = inst->src1_index;
    int src2 = inst->src2_index_imm;
    int dst = inst->dst_index;
    int idx1 = multithread->regs_array[running_thread].reg[src1];
    int idx2;
    if (inst->isSrc2Imm){
        idx2 = src2;
    } else {
        idx2 = multithread->regs_array[running_thread].reg[src2];
    }
    if (is_load){
        // dst <- Mem[src1 + src2]  (src2 may be an immediate)
        SIM_MemDataRead((idx1 +idx2), &multithread->regs_array[running_thread].reg[dst]);
        //printf("LOAD reg%d <- MEM[%d] \n", dst, idx1 + idx2);
        if (multithread->load_cycles > 0) {
            multithread->map_thread[running_thread]->stat_thread = WAITING;
            multithread->map_thread[running_thread]->countdown_thread = multithread->load_cycles;
        }
        // else we dont need to do anything because the thread isn't waiting
    }
    else {
        // Mem[dst + src2] <- src1  (src2 may be an immediate)
        SIM_MemDataWrite((dst + idx2), multithread->regs_array[running_thread].reg[idx1]);
        //printf("STORE MEM[%d] <- %d \n", dst+ idx2, multithread->regs_array[running_thread].reg[idx1]);
        if (multithread->store_cycles > 0) {
            multithread->map_thread[running_thread]->stat_thread = WAITING;
            multithread->map_thread[running_thread]->countdown_thread = multithread->store_cycles;
        }
        // else we dont need to do anything because the thread isn't waiting
    }
    /*for (int i=0; i<REGS_COUNT; ++i)
        printf("\tR%d = 0x%X ", i, multithread->regs_array[running_thread].reg[i]);
        */
}

void updateThreadsQueueStatus(ThreadsStatus* multithread) {
    for (int i = 0; i < multithread->num_threads; i++) {
        if (multithread->map_thread[i]->stat_thread == WAITING) {
            multithread->map_thread[i]->countdown_thread--;
            if (multithread->map_thread[i]->countdown_thread <= 0) {
                multithread->map_thread[i]->stat_thread = READY;
            }
        }
    }
}


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
   for (int i = 0; i < threads_num ; i++) {
       threads_queue.push(i);
   }

   while(!threads_queue.empty()) {
       int running_thread = threads_queue.front();
       Instruction new_inst;
       while (blocked_multithread->map_thread[running_thread]->stat_thread == READY) { // while thread can still run
           blocked_multithread->num_instructions++;
           blocked_multithread->total_cycles++;
           SIM_MemInstRead(blocked_multithread->map_thread[running_thread]->cur_line, &new_inst, running_thread);
           switch (new_inst.opcode) {
               case CMD_NOP:
                   blocked_multithread->total_cycles++;
                   break;
               case CMD_ADD:
                   addOrSubOperation(blocked_multithread, running_thread, &new_inst, true);
                   break;
               case CMD_SUB:
                   addOrSubOperation(blocked_multithread, running_thread, &new_inst, false);
                   break;
               case CMD_ADDI:
                   addOrSubOperation(blocked_multithread, running_thread, &new_inst, true);
                   break;
               case CMD_SUBI:
                   addOrSubOperation(blocked_multithread, running_thread, &new_inst, false);
                   break;
               case CMD_LOAD:
                   loadOrStoreOperation(blocked_multithread, running_thread, &new_inst, true);
                   break;
               case CMD_STORE:
                   loadOrStoreOperation(blocked_multithread, running_thread, &new_inst, false);
                   break;
               case CMD_HALT:
                   blocked_multithread->map_thread[running_thread]->stat_thread = HALT;
                   threads_queue.pop(); // TODO : not sure it needs to be here
                   break;
               default:
                   break;
           }
           blocked_multithread->map_thread[running_thread]->cur_line++;
           for (int i = 0; i < blocked_multithread->num_threads; i++) {
               if (blocked_multithread->map_thread[i]->stat_thread == WAITING && (i != running_thread)) {
                   blocked_multithread->map_thread[i]->countdown_thread--;
                   if (blocked_multithread->map_thread[i]->countdown_thread <= 0) {
                       blocked_multithread->map_thread[i]->stat_thread = READY;
                   }
               }
           }
       }
       // Current stat_thred != READY. check if this thread can't run but others can
       if (blocked_multithread->readyThreads()) {
           //&& (blocked_multithread->map_thread[running_thread]->stat_thread != READY) - DONT NEED BECAUSE WE ARE OUT OF THE WHILE LOOP
           blocked_multithread->total_cycles += blocked_multithread->context_switch_cost;
           for (int i = 0; i < blocked_multithread->num_threads; i++) {
               if (blocked_multithread->map_thread[i]->stat_thread == WAITING) {
                   blocked_multithread->map_thread[i]->countdown_thread -= blocked_multithread->context_switch_cost;
                   if (blocked_multithread->map_thread[i]->countdown_thread <= 0) {
                       blocked_multithread->map_thread[i]->stat_thread = READY;
                   }
               }
           }
           if (blocked_multithread->map_thread[running_thread]->stat_thread != HALT) { //meaning we're WAITING
               /*
                * we are not removing elements from queue unless its halted
                * ( in order to try working on the same thread that we are on it now)
                */
               threads_queue.pop();
               threads_queue.push(running_thread);
           }
       } else { // idle - no thread can run
           blocked_multithread->total_cycles++;
           //todo:check if works fine. I left the original code below.
           updateThreadsQueueStatus(blocked_multithread);
           /*for (int i = 0; i < blocked_multithread->num_threads; i++) {
               if (blocked_multithread->map_thread[i]->stat_thread == WAITING) {
                   blocked_multithread->map_thread[i]->countdown_thread--;
                   if (blocked_multithread->map_thread[i]->countdown_thread <= 0) {
                       blocked_multithread->map_thread[i]->stat_thread = READY;
                   }
               }
           }*/
       }
   }
    /*for(int k=0; k<threads_num; k++){
        printf("\nTESTTTTTT Register file thread id %d:\n", k);
        for (int i=0; i<REGS_COUNT; ++i) {
            printf("\tR%d = 0x%X", i, blocked_multithread->regs_array[k].reg[i]);
        }
        printf("\n");
    }*/
}

/*
 *This function contains a full simulation of a fine-grained MT machine.
 * The function reaches it's end when all of the threads reach status 'HALT'.
 */
void CORE_FinegrainedMT() {
    int threads_num = SIM_GetThreadsNum();
    if (threads_num <= 0){
        return;
    }
    finegrained_multithread = new ThreadsStatus(threads_num, SIM_GetLoadLat(), SIM_GetStoreLat(),
            SIM_GetSwitchCycles());

    auto* temp = finegrained_multithread;

    /* creating a queue of threads and filling it up */
    std::queue<int> threads_queue; //todo: check if we should include thread number 0 time
    for (int i = 0; i < threads_num; i++) {
        threads_queue.push(i);
    }

    while (!threads_queue.empty()){
        /*if (finegrained_multithread->total_cycles == 6){
            std::cout << std::endl;
        }*/
        int running_thread = threads_queue.front();
        Instruction new_inst;
        SIM_MemInstRead(blocked_multithread->map_thread[running_thread]->cur_line, &new_inst, running_thread);
        bool current_command_is_halt = false;
        /* if current thread STATUS = READY -> run it */
        if (finegrained_multithread->map_thread[running_thread]->stat_thread == READY) {
            finegrained_multithread->num_instructions++;
            finegrained_multithread->total_cycles++;
            switch (new_inst.opcode){
                case CMD_NOP:
                    blocked_multithread->total_cycles++;
                    break;
                case CMD_ADD:
                    addOrSubOperation(finegrained_multithread, running_thread, &new_inst, true);
                    break;
                case CMD_SUB:
                    addOrSubOperation(finegrained_multithread, running_thread, &new_inst, false);
                    break;
                case CMD_ADDI:
                    addOrSubOperation(finegrained_multithread, running_thread, &new_inst, true);
                    break;
                case CMD_SUBI:
                    addOrSubOperation(finegrained_multithread, running_thread, &new_inst, false);
                    break;
                case CMD_LOAD:
                    loadOrStoreOperation(finegrained_multithread, running_thread, &new_inst, true);
                    break;
                case CMD_STORE:
                    loadOrStoreOperation(finegrained_multithread, running_thread, &new_inst, false);
                    break;
                case CMD_HALT:
                    finegrained_multithread->map_thread[running_thread]->stat_thread = HALT;
                    threads_queue.pop();
                    current_command_is_halt = true;
                    break;
                default:
                    break;
            }

            finegrained_multithread->map_thread[running_thread]->cur_line++;

            /* updating the status of all threads in the queue */
            for (int i = 0; i < finegrained_multithread->num_threads; i++) {
                if (finegrained_multithread->map_thread[i]->stat_thread == WAITING && (i != running_thread)) {
                    finegrained_multithread->map_thread[i]->countdown_thread--;
                    if (finegrained_multithread->map_thread[i]->countdown_thread <= 0) {
                        finegrained_multithread->map_thread[i]->stat_thread = READY;
                    }
                }
            }
            /* this thread finished running - switch to the next available thread in line */
            if (!current_command_is_halt) { //if it's Halt - we already removed it.
                if (finegrained_multithread->readyThreads()) { //meaning there are other ready threads
                    threads_queue.pop();
                    threads_queue.push(running_thread);
                }
            }
            //continue; //start the loop again.
            //NOTE: I added 'else' to the next condition to avoid usage of continue
        }

        /* when reaching here - current thread isn't ready to run
         * check if there's another thread that's READY */

        else if (finegrained_multithread->readyThreads()) {
            threads_queue.pop();
            threads_queue.push(running_thread);
        } else { /* when reaching here - there are no available threads. we're idle. */
            finegrained_multithread->total_cycles++;
            updateThreadsQueueStatus(finegrained_multithread);
            if (finegrained_multithread->readyThreads()) { //meaning there are other ready threads
                //if (finegrained_multithread->map_thread[running_thread]->stat_thread != READY) {
                    threads_queue.pop();
                    threads_queue.push(running_thread);
                //}
            }
        }
    }
}

/*
 * calculated the performence of the system in CPI.
 * @return the performence fo the system in CPI units
 */
double CORE_BlockedMT_CPI(){
    double cpi = (double)blocked_multithread->total_cycles / (double)blocked_multithread->num_instructions;
    delete blocked_multithread;
    return cpi;
}

/*
 * calculated the performence of the system in CPI.
 * @return the performence fo the system in CPI units
 */
double CORE_FinegrainedMT_CPI(){
    /*std::cout << "total cycles: " << (double)finegrained_multithread->total_cycles << std::endl;
    std::cout << "number of instructions: " << (double)finegrained_multithread->num_instructions << std::endl;*/
    double cpi = (double)finegrained_multithread->total_cycles / (double)finegrained_multithread->num_instructions;
    delete finegrained_multithread;
    return cpi;
}

/*
 *Function returns the register file of a given thread through a pointer named context
 * @param context
 * @param threadid - a given id of a thread
 */
void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
    for (int i=0; i<REGS_COUNT; ++i) {
        context[threadid].reg[i] = blocked_multithread->regs_array[threadid].reg[i];
    }
}

/*
 *Function returns the register file of a given thread through a pointer named context
 * @param context
 * @param threadid - a given id of a thread
 */
void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
    for (int i=0; i<REGS_COUNT; ++i) {
        context[threadid].reg[i] = finegrained_multithread->regs_array[threadid].reg[i];
    }
}