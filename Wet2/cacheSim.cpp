/* 046267 Computer Architecture - Spring 2021 - HW #2 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <utility>
#include<bits/stdc++.h>
#include <algorithm>


using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;

typedef enum {NO_WRITE_ALLOCATE, WRITE_ALLOCATE} policy;
typedef enum {L1, L2} cache_name;
typedef enum {DOESNT_EXIST = 0 , EXIST = 1, DIRTY = 2} data_status;


/**
 * the following class simulates how the work of a cache
 */
class Cache {
public:
    unsigned size_of_cache;
    unsigned associative_level;
    unsigned block_size;
    unsigned num_of_rows; // size_of_cache /(block_size * associative_level)
    int access_count = 0;
    std::vector< std::vector<std::pair<unsigned, data_status>>> data; //pair : tag, status
    std::vector< std::vector<unsigned> > LRU_vector; //each vector represents a line of cache. each inner vector represents a cell (depends on associative level)

    Cache(unsigned cache_size, unsigned associative_level, unsigned block_size): size_of_cache(pow(2,10)*cache_size),
                associative_level(associative_level), block_size(block_size){
        num_of_rows = size_of_cache /(block_size * associative_level);
        data.resize(num_of_rows, std::vector<std::pair<unsigned, data_status>>(associative_level,
                std::make_pair(0,DOESNT_EXIST)));
        LRU_vector.resize(num_of_rows, std::vector<unsigned int>(0, 0));
    }
    ~Cache() = default;
    Cache(Cache& other) = default;
    bool in_cache(unsigned long int address, std::pair<unsigned, data_status>* return_pair);
    bool add(unsigned long int address, const unsigned* LRU_address);
    void changeToX(unsigned long int address, data_status new_data_status);
    bool checkIfDirty(unsigned long int address);

    /* LRU functions */
    // NOTE: back of vector = most used. front of vector = least used
    unsigned* LRUgetLeastRecentlyUsed(unsigned long address);
    void LRUupdate(unsigned long address);
    void LRUremove(unsigned long address);
};

bool Cache::in_cache(unsigned long int address, std::pair<unsigned, data_status>* return_pair) {
    if (return_pair == nullptr) return false;
    unsigned int offset_size = log2(this->block_size); // 8 is the num of bits in byte
    unsigned long int tag = address >> offset_size; // get the upper bits of the address to check with tag
    unsigned set = tag%this->num_of_rows;
    for (unsigned int i = 0; i < this->associative_level ; ++i) {
        if ((this->data[set][i].first == tag) && (this->data[set][i].second != DOESNT_EXIST)) {
            *return_pair = this->data[set][i];
            return true;
        }
    }
    return_pair = nullptr;
    return false;
}

// returns id evacuation is needed
bool Cache::add(unsigned long address, const unsigned* LRU_address) {
    bool need_evac = false;
    unsigned int offset_size = log2(this->block_size); // 8 is the num of bits in byte
    unsigned long int tag = address >> offset_size; // get the upper bits of the address to check with tag  TODO : check !! if from LRU/regular address
    unsigned set = tag%this->num_of_rows;

    for (auto &data_address :  this->data[set]){ // cannot have both -> if lru_address == 0 ( does not exist) then will not enter due to data_address.second == EXIST
        // need to replace LRU_address with address  todo: add check if LRU exists
        if (LRU_address != nullptr) {
            unsigned long int tag_LRU = *LRU_address >> offset_size; // get the upper bits of the address to check with tag
            if (data_address.first == tag_LRU && data_address.second != DOESNT_EXIST){
                if (data_address.second == DIRTY) { // evacuate from L1
                    need_evac = true;
                }
                data_address.first = tag;
                data_address.second = EXIST;
                break;
            }
        }
        // need to find a free location in N-ways
        if (data_address.second == DOESNT_EXIST){ // add the data to the available spot
            data_address.first = tag;
            data_address.second = EXIST;
            break;
        }
    }
    return need_evac;
}

/**
 * function changes the status of a given adress to the given status (Exist, doesnt_exist or dirty)
 * @param address
 * @param new_data_status
 */
void Cache::changeToX(unsigned long address, data_status new_data_status) {
    unsigned int offset_size = log2(this->block_size); // 8 is the num of bits in byte
    unsigned long int tag = address >> offset_size;
    unsigned set = tag%this->num_of_rows;
    for (unsigned int i = 0; i < this->associative_level ; ++i) {
        if (this->data[set][i].first == tag) {
            this->data[set][i].second = new_data_status;
        }
    }
}

bool Cache::checkIfDirty(unsigned long address) {
    unsigned int offset_size = log2(this->block_size); // 8 is the num of bits in byte
    unsigned long int tag = address >> offset_size;
    unsigned set = tag%this->num_of_rows;
    for (unsigned int i = 0; i < this->associative_level ; ++i) {
        if (this->data[set][i].first == tag) {
            return (this->data[set][i].second == DIRTY);
        }
    }
    return false;
}

/**
 * returns address of least recently used value
 * @param address
 * @return
 */
unsigned* Cache::LRUgetLeastRecentlyUsed(unsigned long address) { //TODO: make sure returns from right end of the vector
    unsigned int offset_size = log2(this->block_size); // 8 is the num of bits in byte
    unsigned long int tag = address >> offset_size; // get the upper bits of the address to check with tag
    unsigned set = tag % (this->num_of_rows);
    if (LRU_vector.at(set).empty()) {
        return nullptr;
    }
    return &(LRU_vector.at(set).at(0));
}

void Cache::LRUremove(unsigned long address) {
    unsigned int offset_size = log2(this->block_size); // 8 is the num of bits in byte
    unsigned long int tag = address >> offset_size; // get the upper bits of the address to check with tag
    unsigned set = tag % (this->num_of_rows);
    bool exist_in_vector = false;
    auto itr = LRU_vector.at(set).begin();
    for (; itr != LRU_vector.at(set).end() ; ++itr) {
        if (*itr == address) {
            exist_in_vector = true;
            break;
        }
    }
    if (exist_in_vector) { //remove from vector
        LRU_vector.at(set).erase(itr);
    }
}

/**
 * checks if given address exists in LRU table.
 * @param address
 * @return
 */
void Cache::LRUupdate(unsigned long address) {
    unsigned int offset_size = log2(this->block_size); // 8 is the num of bits in byte
    unsigned long int tag = address >> offset_size; // get the upper bits of the address to check with tag
    unsigned set = tag % (this->num_of_rows);
    bool exist_in_vector = false;
    auto itr = LRU_vector.at(set).begin();
    for (; itr != LRU_vector.at(set).end() ; ++itr) {
        if (*itr == address) {
            exist_in_vector = true;
            break;
        }
    }
    //this for loop does the same, while using index instead of iterators
    /*int index;
    for (index = 0; index < LRU_vector->at(set).size(); ++index) {//search in the relevant line
        if (LRU_vector->at(set).at(index) == address) {
            exist_in_vector = true;
            break;
        }
    }*/
    //now index is the location of the corresponding col.
    if (exist_in_vector) { //remove from vector
        LRU_vector.at(set).erase(itr);
    }
    LRU_vector.at(set).push_back(address);
}

/**
 * this class manages the interaction with the memory.
 * it contains two field of class 'Cache', each one represents a cache.
 */
class Memory {
public:
    Cache* L1_cache;
    Cache* L2_cache;
    policy cache_policy;
    unsigned access_count_L1 = 0;
    unsigned miss_count_L1 = 0;
    unsigned access_count_L2 = 0;
    unsigned miss_count_L2 = 0;
    unsigned access_count_mem = 0;
    unsigned dram_cycles;
    unsigned L1_cycles;
    unsigned L2_cycles;
    unsigned block_size;
    std::vector< std::pair<unsigned, data_status> > LRU;
    Memory(unsigned MemCyc,unsigned BSize, unsigned L1Size, unsigned L2Size, unsigned L1Assoc,
           unsigned L2Assoc, unsigned L1Cyc, unsigned L2Cyc, unsigned WrAlloc) :
           dram_cycles(MemCyc), L1_cycles(L1Cyc),L2_cycles(L2Cyc), block_size(pow(2,BSize)){ // c'tor
        L1_cache = new Cache(pow(2,L1Size), pow(2, L1Assoc), pow(2,BSize));
        L2_cache = new Cache(pow(2,L2Size), pow(2, L2Assoc), pow(2,BSize));
        cache_policy = WrAlloc ? WRITE_ALLOCATE : NO_WRITE_ALLOCATE;
    }
    ~Memory() {
        delete L1_cache;
        delete L2_cache;
    }
    Memory(Memory& other) = default;
    void calc_operation(unsigned long int address, char op);
};

void Memory::calc_operation(unsigned long int address, char op) {
    unsigned data_location = address % (this->block_size);
    std::pair <unsigned, data_status> returned_pair;
    access_count_L1++; // add the time to access L1 cache
    if (this->L1_cache->in_cache(data_location, &returned_pair) == true) {
        L1_cache->LRUupdate(address);
        if(op == 'w') { // need to specify as dirty
            L1_cache->changeToX(address, DIRTY);
        }
    }
    else { // not is L1 cache -> check in L2
        this->miss_count_L1++;
        access_count_L2++;
        if (this->L2_cache->in_cache(data_location, &returned_pair) == true) { // in L2
            if (((op == 'w') && (this->cache_policy == WRITE_ALLOCATE)) || op == 'r'){ // need to add address to L1
                unsigned* LRU_address;  // TODO : + get LRU address (use it only if remove is needed) : insert address
                LRU_address = L1_cache->LRUgetLeastRecentlyUsed(address);
                if (L1_cache->add(address, LRU_address) &&  ((LRU_address != nullptr) && L1_cache->checkIfDirty(*LRU_address))){ // if need to evict old address
                    L2_cache->LRUupdate(*LRU_address);
                    L1_cache->LRUremove(*LRU_address);
                    L2_cache->changeToX(address, DIRTY); // basically we don't check dirty of L2 so don't need!!
                }
                if (op == 'w') {
                    L1_cache->changeToX(address, DIRTY);
                }
                if (op == 'r') {
                    L1_cache->changeToX(address, EXIST);
                    L2_cache->changeToX(address, EXIST);
                }
                L1_cache->LRUupdate(address);
                L2_cache->LRUupdate(address);
            }
            else { // ((op == 'w') && (this->cache_policy == NO_WRITE_ALLOCATE))
                access_count_mem++; // update mem with the data
                L2_cache->LRUupdate(address);
                L2_cache->changeToX(address, DIRTY);
            }
        }
        else { // not in L2
            miss_count_L2++;
            access_count_mem++;
            if (((op == 'w') && (this->cache_policy == WRITE_ALLOCATE)) || op == 'r'){ // need to add address to L1 and L2
                unsigned* LRU_address;
                LRU_address = L1_cache->LRUgetLeastRecentlyUsed(address);
                if (L1_cache->add(address, LRU_address) && ((LRU_address != nullptr) && L1_cache->checkIfDirty(*LRU_address)) ){ // if need to evict old address
                    L2_cache->LRUupdate(*LRU_address);
                    L1_cache->LRUremove(*LRU_address);
                }
                if (L2_cache->add(address, LRU_address)) { // TODO : + get LRU address
                    L2_cache->LRUremove(*LRU_address); // check if not in L1 ?????
                    //L1_cache->LRUremove(*LRU_address); //TODO: make sure it is removed when reaching here
                }
                L1_cache->LRUupdate(address);
                L2_cache->LRUupdate(address);
            }
             //else :  // ((op == 'w') && (this->cache_policy == NO_WRITE_ALLOCATE))
             // go to mem and update the data
             // do not update LRU at all
        }
    }
}

int main(int argc, char **argv) {
    std::cout << "number of arguments: " << argc << std::endl;
	if (argc < 19) {
		cerr << "Not enough arguments" << endl;
		return 0;
	}

	// Get input arguments

	// File
	// Assuming it is the first argument
	char* fileString = argv[1];
	ifstream file(fileString); //input file stream
	string line;
	if (!file || !file.good()) {
		// File doesn't exist or some other error
		cerr << "File not found" << endl;
		return 0;
	}

	unsigned MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
			L2Assoc = 0, L1Cyc = 0, L2Cyc = 0, WrAlloc = 0;

	for (int i = 2; i < 19; i += 2) {
		string s(argv[i]);
		if (s == "--mem-cyc") {
			MemCyc = atoi(argv[i + 1]);
		} else if (s == "--bsize") {
			BSize = atoi(argv[i + 1]);
		} else if (s == "--l1-size") {
			L1Size = atoi(argv[i + 1]);
		} else if (s == "--l2-size") {
			L2Size = atoi(argv[i + 1]);
		} else if (s == "--l1-cyc") {
			L1Cyc = atoi(argv[i + 1]);
		} else if (s == "--l2-cyc") {
			L2Cyc = atoi(argv[i + 1]);
		} else if (s == "--l1-assoc") {
			L1Assoc = atoi(argv[i + 1]);
		} else if (s == "--l2-assoc") {
			L2Assoc = atoi(argv[i + 1]);
		} else if (s == "--wr-alloc") {
			WrAlloc = atoi(argv[i + 1]);
		} else {
			cerr << "Error in arguments" << endl;
			return 0;
		}
	}
    string address;
    char operation = 0; // read (R) or write (W)
    unsigned long int num = 0;
    auto cpu_mem = new Memory(MemCyc,BSize, L1Size, L2Size, L1Assoc, L2Assoc, L1Cyc, L2Cyc,WrAlloc );

	while (getline(file, line)) {

		stringstream ss(line);

		if (!(ss >> operation >> address)) {
			// Operation appears in an Invalid format
			cout << "Command Format error" << endl;
			return 0;
		}

		// DEBUG - remove this line
		cout << "operation: " << operation;

		string cutAddress = address.substr(2); // Removing the "0x" part of the address

		// DEBUG - remove this line
		cout << ", address (hex)" << cutAddress;


		num = strtoul(cutAddress.c_str(), NULL, 16);

		// DEBUG - remove this line
		cout << " (dec) " << num << endl;
        cpu_mem->calc_operation(num, operation);
	}
    double L1MissRate = 0;
    double L2MissRate = 0;
    double avgAccTime = 0;

    avgAccTime = (cpu_mem->access_count_L1*cpu_mem->L1_cycles + cpu_mem->access_count_L2*cpu_mem->L2_cycles +cpu_mem->access_count_mem*cpu_mem->dram_cycles)/
            (cpu_mem->access_count_L1);

    if (cpu_mem->access_count_L1 != 0) {
        L1MissRate = cpu_mem->miss_count_L1 / cpu_mem->access_count_L1;
    }
    std::cout << "L1 Miss rate: " << L1MissRate << std::endl;

    if (cpu_mem->access_count_L2 != 0){
        L2MissRate = cpu_mem->miss_count_L2 / cpu_mem->access_count_L2;
    }
    std::cout << "L2 Miss rate: " << L2MissRate << std::endl;

    printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	delete cpu_mem;
	return 0;
}