/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <map>
#include <iostream>
#include <vector>
#include "stdio.h"
#include "math.h"

enum BIMODAL{USING_SHARE_LSB = 1, USING_SHARE_MID = 2, NOT_USING_SHARE=0 };
enum SHARE_TYPE{GLOBAL, LOCAL};
const int MAX_BTB_SIZE = 32;
const int  MAX_STATE_MACHINES = 256; // 2^8 (8 = max size of history)
const int COLUMNS_BTB = 4; // for [0] = valid bit ,[1] = tag and [2] = history, [3] = jmp_ip
enum FSM_STATE{SNT = 0, WNT = 1, WT = 2, ST = 3};

class FSM {
private:
    FSM_STATE current_state;
public:
    FSM(uint32_t initial_state):
        current_state(FSM_STATE(initial_state))
    {}
    bool getState(){
        return (current_state >= 2);
    }
    FSM_STATE &operator++();
    FSM_STATE &operator--();
};

class BP {
public:
    uint32_t BTB_size;
    uint32_t history_reg_size;
    uint32_t tag_size;
    BIMODAL using_share_type;
    FSM start_state;
    SHARE_TYPE history_type;
    SHARE_TYPE state_machine_type;
    std::vector<std::vector<uint32_t>> history_cache;
    uint32_t global_history = 0;
    std::vector<std::vector<FSM>> local_state_machine_array;
    std::vector<FSM> global_state_machine_array;
    int branch_counter = 0;
    int wrong_prediction_counter = 0;


    // functions
    BP(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
       bool isGlobalHist, bool isGlobalTable, int Shared) :
       BTB_size(btbSize),
       history_reg_size(historySize),
       tag_size(tagSize),
       using_share_type(BIMODAL(Shared)),
       start_state(FSM_STATE(fsmState))
       {
        history_type = (isGlobalHist ? GLOBAL : LOCAL);
        state_machine_type = (isGlobalTable ? GLOBAL : LOCAL);

    } //c'tor
    ~BP() = default; // d'tor
    BP(BP& other) = default; // copy c'tor
};

BP* bp_pointer;

FSM_STATE& FSM::operator++(){
    if (current_state == ST){
        return current_state;
    }
    else if (current_state == SNT) {
        current_state = WNT;
        return current_state;
    }
    else if (current_state == WNT){
        current_state = WT;
        return current_state;
    }
    else{       //if (current_state == WT)
        current_state = ST;
        return current_state;
    }
}

FSM_STATE& FSM::operator--(){
    if (current_state == SNT){
        return current_state;
    }
    else if (current_state == WNT) {
        current_state = SNT;
        return current_state;
    }
    else if (current_state == WT){
        current_state = WNT;
        return current_state;
    }
    else{       //if (current_state == ST)
        current_state = WT;
        return current_state;
    }
}
uint32_t create_align(uint32_t size){
    uint32_t align = 0;
    for (int i = 0 ; i < log2(size); i ++){
        align |= (1 << i);
    }
    return align;
}

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
    BP* branch_predictor = new BP(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
    if (branch_predictor == nullptr){
        return -1; // check if new failed
    }
    bp_pointer = branch_predictor;

    uint32_t num_state_machines = pow(2,bp_pointer->history_reg_size);

    if (bp_pointer->state_machine_type == LOCAL) { // state machines are LOCAL
        bp_pointer->local_state_machine_array.resize(bp_pointer->BTB_size, std::vector<FSM>(num_state_machines, FSM(bp_pointer->start_state)));
    }
    else { // state machines are GLOBAL
        bp_pointer->global_state_machine_array.resize(num_state_machines,FSM(bp_pointer->start_state));
    }

    if (bp_pointer->history_type == LOCAL) { //private history
        bp_pointer->history_cache.resize(bp_pointer->BTB_size, std::vector<uint32_t>(COLUMNS_BTB, 0));
    }
    // else shared history - dont need to do anything - we have global_history
    return 0;
}

bool check_lh_lfsm(uint32_t pc, uint32_t *dst){
    uint32_t btb_row_align = create_align(bp_pointer->BTB_size);
    uint32_t tag_align =  create_align(bp_pointer->tag_size);
    uint32_t history_cache_row = (pc >> 2) & btb_row_align;
    uint32_t pc_tag = ((pc >> (int(log2(bp_pointer->BTB_size)) + 2)) & tag_align);
    uint32_t table_tag = bp_pointer->history_cache[history_cache_row][1];

    if (bp_pointer->history_cache[history_cache_row][0] && table_tag == pc_tag) { // branch exists
        uint32_t sm_column = bp_pointer->history_cache[history_cache_row][2];
        if (bp_pointer->local_state_machine_array[history_cache_row][sm_column].getState()) {
            *dst = bp_pointer->history_cache[history_cache_row][3]; // put dst in the pointer
            return true; // if got here -> state is Taken so we return True
        }
    }
    // predicted NT or not found in table
    *dst = pc +4;
    return false;
}

bool check_lh_gfsm(uint32_t pc, uint32_t *dst){
    uint32_t btb_row_align = create_align(bp_pointer->BTB_size);
    uint32_t tag_align = create_align(bp_pointer->tag_size);

    uint32_t history_cache_row = (pc >> 2) & btb_row_align;
    uint32_t pc_tag = ((pc >> (int(log2(bp_pointer->BTB_size)) + 2)) & tag_align);
    uint32_t table_tag = bp_pointer->history_cache[history_cache_row][1];
    if (bp_pointer->history_cache[history_cache_row][0] && table_tag == pc_tag) { // branch exists
        uint32_t history_fsm_row = 0;
        // check what row to get in global history FSM
        if (bp_pointer->using_share_type == USING_SHARE_LSB) {
            uint32_t share_lsb = create_align(bp_pointer->history_reg_size);
            history_fsm_row = (pc >> 2) & share_lsb;
        }
        if (bp_pointer->using_share_type == USING_SHARE_MID) {
            uint32_t share_lsb = create_align(bp_pointer->history_reg_size);
            history_fsm_row = (pc >> 16) & share_lsb;
        }
        else { // not using share
            history_fsm_row = bp_pointer->history_cache[history_cache_row][2];
        }
        //check FSM state
        if(bp_pointer->global_state_machine_array[history_fsm_row].getState()){
            *dst = bp_pointer->history_cache[history_cache_row][3]; // put dst in the pointer
            return true; // if got here -> state is Taken so we return True
        }
    }
    // predicted NT or not found in table
    *dst = pc +4;
    return false;
}

bool check_gh_gfsm(uint32_t pc, uint32_t *dst) {
    // #############################################################################
    // TODO : ASK Lina !! how do we search in the BT ? do we need to check if the branch exists in the table ?
    // #############################################################################
    uint32_t btb_row_align = create_align(bp_pointer->BTB_size);
    uint32_t tag_align = create_align(bp_pointer->tag_size);

    uint32_t history_cache_row = (pc >> 2) & btb_row_align;
    uint32_t pc_tag = ((pc >> (int(log2(bp_pointer->BTB_size)) + 2)) & tag_align);
    uint32_t table_tag = bp_pointer->history_cache[history_cache_row][1];

    if (bp_pointer->history_cache[history_cache_row][0] && table_tag == pc_tag){ // if found in table
        if(bp_pointer->global_state_machine_array[bp_pointer->global_history].getState()){ // if Taken
            *dst = bp_pointer->history_cache[history_cache_row][3]; // put dst in the pointer
            return true; // if got here -> state is Taken so we return True
        }
    }
    // predicted NT or not found in table
    *dst = pc +4;
    return false;
}

bool check_gh_lfsm(uint32_t pc, uint32_t *dst){
    uint32_t fsm_row_align = create_align(bp_pointer->BTB_size);
    uint32_t tag_align = create_align(bp_pointer->tag_size);

    uint32_t fsm_row = (pc >> 2) & fsm_row_align;
    uint32_t pc_tag = ((pc >> (int(log2(bp_pointer->BTB_size)) + 2)) & tag_align);
    uint32_t table_tag = bp_pointer->history_cache[fsm_row][1];

    if (bp_pointer->history_cache[fsm_row][0] && table_tag == pc_tag){ // if found in table
        if(bp_pointer->local_state_machine_array[fsm_row][bp_pointer->global_history].getState()){ // if Taken
            *dst = bp_pointer->history_cache[fsm_row][3]; // put dst in the pointer
            return true; // if got here -> state is Taken so we return True
        }
    }
    // predicted NT or not found in table
    *dst = pc +4;
    return false;
}

bool BP_predict(uint32_t pc, uint32_t *dst){

    if (bp_pointer->history_type == LOCAL && bp_pointer->state_machine_type == LOCAL){
        return check_lh_lfsm(pc, dst);
        // TODO : if btb_size =1
        //TODO: call update ? how do we check if we predicted right ???
    }
    else if (bp_pointer->history_type == LOCAL && bp_pointer->state_machine_type == GLOBAL){
        return check_lh_gfsm(pc, dst);
    }
    else if (bp_pointer->history_type == GLOBAL && bp_pointer->state_machine_type == GLOBAL){
        return check_gh_gfsm(pc, dst);
    }
    else if (bp_pointer->history_type == GLOBAL && bp_pointer->state_machine_type == LOCAL){
        return check_gh_lfsm(pc, dst);
    }
    // not valid arguments
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	return;
}

void BP_GetStats(SIM_stats *curStats){
    /**
     * branch_predictor_pointer is a global variable of class BP. the variable contains
     * parameters named 'branch_counter' and 'wrong_prediction_counter' that are updated
     * during runtime.
     */
    curStats->br_num = bp_pointer->branch_counter;
    curStats->flush_num = bp_pointer->wrong_prediction_counter;

    /**
     * in order to calculate the theoretical allocated size we'll use the following variables.
     * the names of the variable match to the names from the tutorial.
     */
    int temp_size=0;
    int entries = bp_pointer->BTB_size;
    int valid_bit = 1;
    int tag_size = bp_pointer->tag_size;
    int target_size = sizeof(uint32_t); //TODO: consider removing the excess 2 bits.
    int history_size = bp_pointer->history_reg_size;

    /**
     * the following line should be calculated as such in either case (there is no dependency
     * in the type on the history/ state machine (local\global)
     */
    temp_size += entries * (valid_bit + tag_size + target_size);

    //adding the relevant size for local\global state machine
    if (bp_pointer->state_machine_type == LOCAL){
        temp_size += int(entries * 2 * pow(2,history_size));
    }
    else if (bp_pointer->state_machine_type == GLOBAL){
        temp_size += int(2 * pow(2,history_size));
    }

    //adding the relevant size to local\global history
    if (bp_pointer->history_type == LOCAL){
        temp_size += entries * history_size;
    }
    else if (bp_pointer->history_type == GLOBAL){
        temp_size += history_size;
    }
    curStats->size = temp_size;
    delete bp_pointer;
	return;
}