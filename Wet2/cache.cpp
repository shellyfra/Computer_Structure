//
// Created by shell on 2021-05-08.
//
#include <vector>
#include <cmath>


typedef enum {NO_WRITE_ALLOCATE, WRITE_ALLOCATE} policy;
typedef enum {L1, L2} cache_name;
typedef enum {DOESNT_EXIST = 0 , EXIST = 1} data_status;

//class Cache {
//public:
//    unsigned int size_of_cache;
//    unsigned int block_size;
//    unsigned int associative_level;
//    unsigned int num_of_rows; // size_of_cache /(block_size * associative_level)
//    int access_count = 0;
//    std::vector< std::vector<data_status> > data;
//    Cache(unsigned int cache_size, unsigned int block_size, unsigned int associative_level): size_of_cache(cache_size),
//        block_size(block_size), associative_level(associative_level){
//        num_of_rows = size_of_cache /(block_size * associative_level);
//        data.resize(num_of_rows, std::vector<data_status>(associative_level,DOESNT_EXIST));
//    }
//    ~Cache() = default;
//    Cache(Cache& other) = default;
//};
//
//class Memory {
//public:
//    Cache* L1_cache;
//    Cache* L2_cache;
//    policy cache_policy;
//    int access_count_L1 = 0;
//    int access_count_L2 = 0;
//    int access_count_mem = 0;
//    unsigned int dram_cycles;
//    unsigned int L1_cycles;
//    unsigned int L2_cycles;
//    Memory(unsigned MemCyc,unsigned BSize, unsigned L1Size, unsigned L2Size, unsigned L1Assoc,
//           unsigned L2Assoc, unsigned L1Cyc, unsigned L2Cyc, unsigned WrAlloc) : dram_cycles(MemCyc), L1_cycles(L1Cyc),
//           L2_cycles(L2Cyc){ // c'tor
//        // TODO : do pow(2, x) for : block_size, cache_size, associatvie
//        L1_cache = new Cache(pow(2,L1Size), pow(2, BSize), pow(2,L1Assoc));
//        L2_cache = new Cache(pow(2,L2Size), pow(2, BSize), pow(2,L2Assoc));
//        cache_policy = WrAlloc ? WRITE_ALLOCATE : NO_WRITE_ALLOCATE;
//    }
//    ~Memory() {
//        delete L1_cache;
//        delete L2_cache;
//    }
//    Memory(Memory& other) = default;
//};