/* 046267 Computer Architecture - Spring 2021 - HW #2 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <utility>
#include<bits/stdc++.h>


using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;

typedef enum {NO_WRITE_ALLOCATE, WRITE_ALLOCATE} policy;
typedef enum {L1, L2} cache_name;
typedef enum {DOESNT_EXIST = 0 , EXIST = 1} data_status;

class Cache {
public:
    unsigned size_of_cache;
    unsigned associative_level;
    unsigned block_size;
    unsigned num_of_rows; // size_of_cache /(block_size * associative_level)
    int access_count = 0;
    std::vector< std::vector<std::pair<unsigned, data_status>>> data; //pair : tag, status
    Cache(unsigned cache_size, unsigned block_size, unsigned associative_level): size_of_cache(cache_size),
                                     block_size(block_size), associative_level(associative_level){
        num_of_rows = size_of_cache /(block_size * associative_level);
        data.resize(num_of_rows, std::vector<std::pair<unsigned, data_status>>(associative_level,
                std::make_pair(0,DOESNT_EXIST)));
    }
    ~Cache() = default;
    Cache(Cache& other) = default;
    bool in_cache(unsigned int address);
};

class Memory {
public:
    Cache* L1_cache;
    Cache* L2_cache;
    policy cache_policy;
    unsigned block_size;
    unsigned access_count_L1 = 0;
    unsigned access_count_L2 = 0;
    unsigned access_count_mem = 0;
    unsigned dram_cycles;
    unsigned L1_cycles;
    unsigned L2_cycles;
    std::vector< std::pair<unsigned, data_status> > LRU;
    Memory(unsigned MemCyc,unsigned BSize, unsigned L1Size, unsigned L2Size, unsigned L1Assoc,
           unsigned L2Assoc, unsigned L1Cyc, unsigned L2Cyc, unsigned WrAlloc) :
           dram_cycles(MemCyc), L1_cycles(L1Cyc),L2_cycles(L2Cyc), block_size(pow(2,BSize)){ // c'tor
        L1_cache = new Cache(pow(2,L1Size), pow(2, BSize), pow(2,L1Assoc));
        L2_cache = new Cache(pow(2,L2Size), pow(2, BSize), pow(2,L2Assoc));
        cache_policy = WrAlloc ? WRITE_ALLOCATE : NO_WRITE_ALLOCATE;
    }
    ~Memory() {
        delete L1_cache;
        delete L2_cache;
    }
    Memory(Memory& other) = default;
    void calc_operation(unsigned long int address, char op,	double* L1MissRate, double* L2MissRate);
};

void Memory::calc_operation(unsigned long int address, char op, double *L1MissRate, double *L2MissRate) {
    unsigned data_location = address % (this->block_size);
    std::pair <unsigned, data_status> *returned_pair;
    if (this->L1_cache->in_cache(data_location, returned_pair) == true) {
        auto itr = std::find (LRU.begin(), LRU.end(), address);
    }
}

int main(int argc, char **argv) {

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

	}
    double L1MissRate = 0;
    double L2MissRate = 0;
    double avgAccTime = 0;
	Memory* cpu_mem = new Memory(MemCyc,BSize, L1Size, L2Size, L1Assoc, L2Assoc, L1Cyc, L2Cyc,WrAlloc );
	cpu_mem->calc_operation(num, operation, &L1MissRate, &L2MissRate);


	printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	delete cpu_mem;
	return 0;
}