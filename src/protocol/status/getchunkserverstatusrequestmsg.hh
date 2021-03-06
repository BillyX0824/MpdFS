/**
 * getchunkserverstatusrequestmsg.hh
 */

#ifndef __GET_CHUNKSERVER_STATUS_REQUEST_HH__
#define __GET_CHUNKSERVER_STATUS_REQUEST_HH__

#include <string>
#include <vector>
#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from Master
 */

class GetChunkserverStatusRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetChunkserverStatusRequestMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	sockfd			Socket descriptor
	 * @param	chunkserverListRef		vector reference for request chunkserver IDs
	 */

	GetChunkserverStatusRequestMsg (Communicator* communicator, uint32_t chunkserverSockfd, vector<uint32_t>& chunkserverListRef);

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

	void setChunkserverStatus(vector<bool>& statusRef);

	vector<bool>& getChunkserverStatus();

private:
	vector<uint32_t> _chunkserverList;
	vector<bool> _chunkserverStatus;
};

#endif
