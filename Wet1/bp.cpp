/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <map>
#include <vector>
#include "stdio.h"

enum BIMODAL{USING_SHARE_LSB = 1, USING_SHARE_MID = 2, NOT_USING_SHARE=0 };
enum SHARE_TYPE{GLOBAL, LOCAL, NONE};
class BP {
private:
    unsigned int BTB_size;
    unsigned int history_reg;
    unsigned int tag_size;
    BIMODAL bimodal_state;
    SHARE_TYPE history_type;
    SHARE_TYPE state_machine_type;
    SHARE_TYPE predictor_type;

public:
    std::map<unsigned int, unsigned int> history_cache;
    map<unsigned int, std::vector<unsigned int>> local_state_machine_array;
    std::vector<unsigned int> global_state_machine_array;
    unsigned int global_history;
    int branch_counter;
    int wrong_prediction_counter;


    BP(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
       bool isGlobalHist, bool isGlobalTable, int Shared) :
       BTB_size(btbSize), history_reg(historySize), tag_size(tagSize), bimodal_state(BIMODAL(fsmState)){
        history_type = (isGlobalHist ? GLOBAL : LOCAL);
        state_machine_type = (isGlobalTable ? GLOBAL : LOCAL);
        if (Shared == 0){
            predictor_type = NONE;
        } else predictor_type = ((Shared == 1) ? GLOBAL : LOCAL);
    }
};


int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
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

