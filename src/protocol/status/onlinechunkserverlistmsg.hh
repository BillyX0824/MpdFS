#ifndef __ONLINECHUNKSERVERLISTMSG_HH__
#define __ONLINECHUNKSERVERLISTMSG_HH__

#include <vector>
#include "../message.hh"
#include "../../common/onlinechunkserver.hh"
using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class OnlineChunkserverListMsg: public Message {
public:

	OnlineChunkserverListMsg(Communicator* communicator);

	OnlineChunkserverListMsg(Communicator* communicator, uint32_t dstSockfd,
			vector<struct OnlineChunkserver>& onlineChunkserverListRef);

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
	vector<struct OnlineChunkserver>& _onlineChunkserverListRef;
	vector<struct OnlineChunkserver>  _onlineChunkserverList;
};

#endif
