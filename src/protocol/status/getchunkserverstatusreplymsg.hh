/**
 * getchunkserverstatusreplymsg.hh
 */

#ifndef __GET_CHUNKSERVER_STATUS_REPLY_HH__
#define __GET_CHUNKSERVER_STATUS_REPLY_HH__

#include <vector>
#include <string>
#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from Master
 */

class GetChunkserverStatusReplyMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetChunkserverStatusReplyMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	numOfObjs	number of Segments
	 * @param	masterSockfd	Socket descriptor
	 */

	GetChunkserverStatusReplyMsg (Communicator* communicator, uint32_t requestId, uint32_t chunkserverSockfd, 
		const vector<bool> &chunkserverStatusRef);

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
	vector<bool> _chunkserverStatus;
};

#endif
