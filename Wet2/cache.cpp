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
public:
    int access_count = 0;
    std::vector< std::vector<data_status> > data;
};

class memory{
public:
    cache L1;
    cache L2;
};