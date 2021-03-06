#ifndef __PUTBLOCKINITREQUEST_HH__
#define __PUTBLOCKINITREQUEST_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 <<<<<<< HEAD
 * Initiate an block trasfer
 */

class PutBlockInitRequestMsg: public Message {
public:

	PutBlockInitRequestMsg(Communicator* communicator);

	PutBlockInitRequestMsg(Communicator* communicator, uint32_t chunkserverSockfd,
			uint64_t segmentId, uint32_t blockId, uint32_t blockSize,
			uint32_t chunkCount, uint32_t blockIdOfDelta,uint32_t deltaId, DataMsgType dataMsgType, string updateKey);

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

private:
	uint64_t _segmentId;
	uint32_t _blockId;
	uint32_t _blockSize;
	uint32_t _chunkCount;
	uint32_t _blockIdOfDelta;
	uint32_t _deltaId;
	DataMsgType _dataMsgType;
	string _updateKey;
};

#endif
