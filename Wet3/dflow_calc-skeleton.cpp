/* 046267 Computer Architecture - Spring 21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <vector>
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

    Node(unsigned int in_delay, unsigned int opcode, unsigned int dst, unsigned int src1, unsigned int src2):
        father1(nullptr), father2(nullptr), delay(in_delay), cycle_depth(0), point_to_entry(false), original_delay(in_delay)

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
        for (int i = 0; i < inst_array.size(); ++i) {
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
        for (int i = 0; i < numOfInsts; ++i) {
            Node *new_inst = new Node(opsLatency[i], progTrace[i].opcode, progTrace[i].dstIdx, progTrace[i].src1Idx,
                                      progTrace[i].src2Idx);
            out_of_order->inst_array.push_back(new_inst);
            Node *src1 = out_of_order->register_array[progTrace[i].src1Idx];
            Node *src2 = out_of_order->register_array[progTrace[i].src2Idx];
            if (src1 == nullptr && src2 == nullptr) {
                new_inst->point_to_entry = true;
            } else if (src1 != nullptr) {
                new_inst->father1 = src1;
                new_inst->delay += opsLatency[new_inst->command[SRC1]]; //enlarging the latency field with src1 value
            } else if (src2 != nullptr) {
                new_inst->father2 = src2;
                new_inst->delay += opsLatency[new_inst->command[SRC2]]; //enlarging the latency field with src2 value
            }
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
    return -1;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    //if (ctx == nullptr || )
    OOOExe* out_of_order =static_cast<OOOExe*>(ctx);
    Node* node_inst = out_of_order->inst_array[theInst];

    return -1;
}

int getProgDepth(ProgCtx ctx) {
    return 0;
}


