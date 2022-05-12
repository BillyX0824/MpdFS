#ifndef __GET_CHUNKSERVER_LIST_REQUEST_HH__
#define __GET_CHUNKSERVER_LIST_REQUEST_HH__

#include <string>
#include "../message.hh"

#include "../../common/onlinechunkserver.hh"
#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from Master
 */

class GetChunkserverListRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetChunkserverListRequestMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	numOfObjs	number of Segments
	 * @param	masterSockfd	Socket descriptor
	 */

	GetChunkserverListRequestMsg (Communicator* communicator, uint32_t chunkserverSockfd);

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

	void setChunkserverList(vector<struct OnlineChunkserver>& _chunkserverList, 
		vector<struct OnlineChunkserver>& chunkserverList);

	vector<struct OnlineChunkserver>& getChunkserverList();

private:
	vector<struct OnlineChunkserver> _chunkserverList;
};

#endif
