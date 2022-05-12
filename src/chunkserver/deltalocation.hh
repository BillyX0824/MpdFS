#ifndef DELTALOCATION_HH_
#define DELTALOCATION_HH_

struct DeltaLocation {
	uint32_t blockId;
	uint32_t deltaId;
	uint32_t blockIdOfDelta;
	bool isReserveSpace;// if in Reserved space
	offset_length_t offsetLength;
};

#endif
