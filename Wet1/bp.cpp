/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <map>
#include <vector>
#include "stdio.h"

enum BIMODAL{USING_SHARE_LSB = 1, USING_SHARE_MID = 2, NOT_USING_SHARE=0 };
enum SHARE_TYPE{GLOBAL, LOCAL, NONE};
const int MAX_BTB_SIZE = 32;
const int MAX_STATE_MACHINES = 256; // 2^8 (8 = max size of history)
const int COLUMNS_BTB = 3; // for [0] = valid bit ,[1] = tag and [2] = history
enum FSM_STATE{SNT = 0, WNT = 1, WT = 2, ST = 3};

class FSM {
private:
    FSM_STATE current_state;
public:
    FSM(int initial_state):
        current_state(FSM_STATE(initial_state))
    {}
    FSM_STATE getState(){
        return current_state;
    }
    FSM_STATE &operator++();
    FSM_STATE &operator--();
};

class BP {
private:
    unsigned int BTB_size;
    unsigned int history_reg;
    unsigned int tag_size;
    BIMODAL bimodal_state;
    SHARE_TYPE history_type;
    SHARE_TYPE state_machine_type;
    SHARE_TYPE predictor_type;
    int history_cache[MAX_BTB_SIZE][COLUMNS_BTB] = {{0}}; // for [0] = valid bit ,[1] = tag and [2] = history
    unsigned int local_state_machine_array[MAX_BTB_SIZE][MAX_STATE_MACHINES] = {{0}};
    unsigned int global_state_machine_array[MAX_STATE_MACHINES] = {0};
    unsigned int global_history = 0;
    int branch_counter = 0;
    int wrong_prediction_counter = 0;

public:

    BP(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
       bool isGlobalHist, bool isGlobalTable, int Shared) :
       BTB_size(btbSize), history_reg(historySize), tag_size(tagSize), bimodal_state(BIMODAL(fsmState)){
        history_type = (isGlobalHist ? GLOBAL : LOCAL);
        state_machine_type = (isGlobalTable ? GLOBAL : LOCAL);
        if (Shared == 0){
            predictor_type = NONE;
        } else predictor_type = ((Shared == 1) ? GLOBAL : LOCAL);


    } //c'tor
    ~BP() = default; // d'tor
    BP(BP& other) = default; // copy c'tor
};


BP* branch_predictor_pointer;

FSM_STATE& FSM::operator++(){
    if (current_state == ST){
        return current_state;
    }
    else if (current_state == SNT) {
        current_state == WNT;
        return current_state;
    }
    else if (current_state == WNT){
        current_state == WT;
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
        current_state == SNT;
        return current_state;
    }
    else if (current_state == WT){
        current_state == WNT;
        return current_state;
    }
    else{       //if (current_state == ST)
        current_state = WT;
        return current_state;
    }
}

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
    BP* branch_predictor = new BP(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
    branch_predictor_pointer = branch_predictor;


    return -1;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	return;
}

void BP_GetStats(SIM_stats *curStats){
	return;
}

