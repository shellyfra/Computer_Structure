//
// Created by shell on 2021-05-08.
//
#include <vector>


typedef enum {NO_WRITE_ALLOCATE, WRITE_ALLOCATE} policy;
typedef enum {L1, L2} cache_name;
typedef enum {DOESNT_EXIST = 0 , EXIST = 1} data_status;

class cache {
private:
    int size_of_cache;
    int block_size;
    int associative_level;
    int access_time;
    policy cache_policy;
    int num_of_rows; // size_of_chace /(block_size * associative_level)
public:
    int access_count = 0;
    std::vector< std::vector<data_status> > data;
};

class memory{
public:
    cache L1;
    cache L2;
    int access_count_L1 = 0;
    int access_count_L2 = 0;
    int access_count_mem = 0;

};