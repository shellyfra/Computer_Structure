/* 046267 Computer Architecture - Spring 21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <vector>
#define OPCODE 0
#define DST 1
#define SRC1 2
#define SRC2 3

class Node{
public:
    Node* father1;
    Node* father2;
    unsigned int delay;
    unsigned int command [4]; //access through defines above: <opcode> <dst> <src1> <src2>
    unsigned int cycle_depth;
    bool point_to_entry;

    Node(unsigned int in_delay, unsigned int opcode, unsigned int dst, unsigned int src1, unsigned int src2):
        father1(nullptr), father2(nullptr), delay(in_delay), cycle_depth(0), point_to_entry(false)
        {
            command[OPCODE] = opcode;
            command[DST] = dst;
            command[SRC1] = src1;
            command[SRC2] = src2;
        }
};


class OOOExe{ //out of order execution
public:
    Node* register_array [32];
    std::vector <Node*> inst_array;
    //std::vector <Node*> exit; TODO: maybe add back. check if register_array is sufficient
    unsigned int op_latency[];

};


ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    return PROG_CTX_NULL;
}

void freeProgCtx(ProgCtx ctx) {
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    return -1;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    return -1;
}

int getProgDepth(ProgCtx ctx) {
    return 0;
}


