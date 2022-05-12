#ifndef __BLOCKDATA_HH__
#define __BLOCKDATA_HH__

#include <string>
#include <stdint.h>
#include "enums.hh"
#include "../common/blocklocation.hh"

using namespace std;

struct BlockInfo {
	uint64_t segmentId;
	uint32_t blockId;
	uint32_t blockSize;
    vector<BlockLocation> parityVector; // pair of <chunkserverid, blockid>
    vector<offset_length_t> offlenVector;
    BlockType blockType;
    uint32_t ifrom;// Whether it is a read-only file, if it is read-only 1, there is no need to allocate reserved space
    uint32_t blockIdOfDelta;
    uint32_t deltaId;
    // only for update
    CodingScheme codingScheme;
    string codingSetting;
    uint64_t segmentSize;

    BlockInfo() {
        segmentId = 0;
        blockId = 0;
        blockSize = 0;
        segmentSize = 0;
        blockType = DEFAULT_BLOCK_TYPE;
        codingScheme = DEFAULT_CODING;
        codingSetting = "";
        ifrom = 0;// The default is 0, which means it is not read-only and can be modified
        blockIdOfDelta = 0;// Parity delta corresponds to data block ID
        deltaId = 0;
    }
};

struct BlockData {
	struct BlockInfo info;
	char* buf;
};

#endif
