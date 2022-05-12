#ifndef __GET_CHUNKSERVER_LIST_REPLY_HH__
#define __GET_CHUNKSERVER_LIST_REPLY_HH__

#include <vector>
#include <string>
#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"
#include "../../common/onlinechunkserver.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from Master
 */

class GetChunkserverListReplyMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetChunkserverListReplyMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	requestId 	request Id
	 * @param	sockfd	requester's sockfd
	 * @param 	chunkserverList referece to a vector of reply chunkserver list
	 */

	GetChunkserverListReplyMsg (Communicator* communicator, uint32_t requestId, uint32_t sockfd, vector<struct OnlineChunkserver>& chunkserverList);

	/**
	 * Copy values in private variables to protocol message
	 * Serialize protocol message and copy to private variable
	 */

	void prepareProtocolMsg ();

	/**
	 * Override
	 * Parse message from raw buffer
	 * @param buf Raw buffer storing header + protocol + payload
	 */

	void parse (char* buf);

	/**
	 * Override
	 * Execute the corresponding Processor
	 */

	void doHandle ();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol ();


private:
	vector<struct OnlineChunkserver>& _chunkserverListRef;
	vector<struct OnlineChunkserver> _chunkserverList;
};

#endif
