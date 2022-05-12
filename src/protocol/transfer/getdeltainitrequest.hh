#ifndef __GETDELTAINITREQUEST_HH__
#define __GETDELTAINITREQUEST_HH__

#include "../message.hh"
#include "../../common/blockdata.hh"

using namespace std;

class GetDeltaInitRequestMsg: public Message {
public:

	GetDeltaInitRequestMsg(Communicator* communicator);

	GetDeltaInitRequestMsg(Communicator* communicator, uint32_t chunkserverSockfd,
			uint64_t segmentId, uint32_t blockId,
			uint32_t beginDeltaId,uint32_t endDeltaId,DataMsgType dataMsgType);

	/**
	 * Copy values in private variables to protocol message
	 * Serialize protocol message and copy to private variable
	 */

	void prepareProtocolMsg();

	/**
	 * Override
	 * Parse message from raw buffer
	 * @param buf Raw buffer storing header + protocol + payload
	 */

	void parse(char* buf);

	/**
	 * Override
	 * Execute the corresponding Processor
	 */

	void doHandle();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol();

	/*
	 void setBlockSize(uint32_t blockSize);
	 uint32_t getBlockSize();
	 void setChunkCount(uint32_t chunkCount);
	 uint32_t getChunkCount();
	void setRecoveryBlockData(BlockData blockData);
	BlockData getRecoveryBlockData();
	 */

private:
	uint64_t _segmentId;
	uint32_t _blockId;
	uint32_t _beginDeltaId;
	uint32_t _endDeltaId;
	uint32_t _getNum;
	DataMsgType _dataMsgType;
};

#endif
