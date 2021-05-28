/* 046267 Computer Architecture - Spring 21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <vector>
#include <algorithm>
#define OPCODE 0
#define DST 1
#define SRC1 2
#define SRC2 3
#define MAX_REGS 32

class Node{
public:
    Node* father1;
    Node* father2;
    unsigned int delay;
    unsigned int command [4]; //access through defines above: <opcode> <dst> <src1> <src2>
    unsigned int cycle_depth;
    bool point_to_entry;
    unsigned int original_delay;
    unsigned int idx_of_instruction;

    Node(unsigned int in_delay, unsigned int opcode, unsigned int dst, unsigned int src1, unsigned int src2, unsigned int idx):
        father1(nullptr), father2(nullptr), delay(in_delay), cycle_depth(0), point_to_entry(false),
        original_delay(in_delay), idx_of_instruction(idx)

    {
            command[OPCODE] = opcode;
            command[DST] = dst;
            command[SRC1] = src1;
            command[SRC2] = src2;
        }

    ~Node() = default;
    Node (Node& other_node) = default;
};

class OOOExe{ //out of order execution
public:
    Node* register_array [MAX_REGS];
    std::vector <Node*> inst_array;
    //std::vector <Node*> exit; TODO: maybe add back. check if register_array is sufficient
    unsigned int op_latency[MAX_OPS];
    unsigned int num_of_insts;


    OOOExe(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts): num_of_insts(numOfInsts) {
        for (int i = 0; i < MAX_REGS; ++i) {
            register_array[i] = nullptr;
        };
        for (int j = 0; j < MAX_OPS; ++j) {
            op_latency[j] = opsLatency[j];
        }
    }

    ~OOOExe(){
        for (unsigned int i = 0; i < inst_array.size(); ++i) {
            delete inst_array.at(i); // TODO: check if it works.
            inst_array.at(i) = nullptr;
        }
    }
    OOOExe(OOOExe& other) = default;
};

/*  -------------------- END OF CLASSES ---------------------- */

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    try {
        OOOExe *out_of_order = new OOOExe(opsLatency, progTrace, numOfInsts );
        //now we will calculate the dependencies :)
        for (unsigned int i = 0; i < numOfInsts; ++i) {
            Node *new_inst = new Node(opsLatency[progTrace[i].opcode], progTrace[i].opcode, progTrace[i].dstIdx, progTrace[i].src1Idx,
                                      progTrace[i].src2Idx, i);
            out_of_order->inst_array.push_back(new_inst);
            Node *src1 = out_of_order->register_array[progTrace[i].src1Idx];
            Node *src2 = out_of_order->register_array[progTrace[i].src2Idx];
            unsigned int father1_delay = 0;
            unsigned int father2_delay = 0;
            if (src1 == nullptr && src2 == nullptr) {
                new_inst->point_to_entry = true;
            }
            if (src1 != nullptr) {
                new_inst->father1 = src1;
                father1_delay = new_inst->father1->delay; //enlarging the latency field with src1 value
            }
            if (src2 != nullptr) {
                new_inst->father2 = src2;
                father2_delay = new_inst->father2->delay; //enlarging the latency field with src2 value
            }
            new_inst->delay+= std::max(father1_delay, father2_delay);
            out_of_order->register_array[new_inst->command[DST]] = new_inst;
        }
        return (ProgCtx) out_of_order; //if the allocation fails, the returned value will be nullptr.
    }
    catch(std::bad_alloc& e){ //in case the allocation fails
        return PROG_CTX_NULL;
    }
}

void freeProgCtx(ProgCtx ctx) {
    OOOExe* out_of_order =static_cast<OOOExe*>(ctx);
    delete out_of_order;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    if (ctx == nullptr){
        return -1;
    }
    OOOExe* out_of_order =static_cast<OOOExe*>(ctx);
    if ((theInst > out_of_order->num_of_insts) || (theInst < 0)) return -1;

    int inclusive_delay = out_of_order->inst_array[theInst]->delay;
    //return the total delay minus the instruction delay
    return inclusive_delay - out_of_order->inst_array[theInst]->original_delay;

}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    if (ctx == nullptr || src1DepInst == nullptr || src2DepInst == nullptr) return -1;
    OOOExe* out_of_order =static_cast<OOOExe*>(ctx);
    if ((theInst > out_of_order->num_of_insts) || (theInst < 0) ) return -1;

    Node* node_inst = out_of_order->inst_array[theInst];
    *src1DepInst = (node_inst->father1 == nullptr) ? -1 : node_inst->father1->idx_of_instruction;
    *src2DepInst = (node_inst->father2 == nullptr) ? -1 : node_inst->father2->idx_of_instruction;
    return 0;
}

int getProgDepth(ProgCtx ctx) {
    return 0;
}


