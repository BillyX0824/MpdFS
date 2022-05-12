#ifndef __CHUNKSERVERSTARTUPMSG_HH__
#define __CHUNKSERVERSTARTUPMSG_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class ChunkserverStartupMsg: public Message {
public:

	ChunkserverStartupMsg(Communicator* communicator);

	ChunkserverStartupMsg(Communicator* communicator, uint32_t dstSockfd,
			uint32_t chunkserverId, uint32_t capacity, uint32_t loading);
	ChunkserverStartupMsg(Communicator* communicator, uint32_t dstSockfd,
			uint32_t chunkserverId, uint32_t capacity, uint32_t loading, uint32_t ip,
			uint16_t port);

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
	uint32_t _chunkserverId;
	uint32_t _capacity;
	uint32_t _loading;
	uint32_t _chunkserverIp;
	uint16_t _chunkserverPort;
};

#endif
