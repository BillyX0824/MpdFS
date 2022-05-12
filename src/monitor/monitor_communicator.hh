#ifndef __MONITOR_COMMUNICATOR_HH__
#define __MONITOR_COMMUNICATOR_HH__

#include <iostream>
#include <stdint.h>
#include <vector>
#include "../communicator/communicator.hh"
#include "../common/onlinechunkserver.hh"

using namespace std;

/**
 * Extends Communicator class
 * Handles all MONITOR communications
 */

class MonitorCommunicator: public Communicator {
public:

	/**
	 * Constructor
	 */
	MonitorCommunicator();

	/**
	 * Destructor
	 */
	~MonitorCommunicator();

	/**
	 * Action to reply a request from Master for primary CHUNKSERVER list
	 * @param requestId Request ID
	 * @param sockfd Socket ID between the connection
	 * @param primaryList List of selected primary chunkserver IDs 
	 */
	void replyPrimaryList(uint32_t requestId, uint32_t sockfd, 
		vector<uint32_t> primaryList);

	/**
	 * Action to reply a request from CHUNKSERVER for secondary CHUNKSERVER list
	 * @param requestId Request ID
	 * @param sockfd Socket ID between the connection
	 * @param secondaryList List of selected secondary chunkserver IDs 
	 */
	void replySecondaryList(uint32_t requestId, uint32_t sockfd, 
		vector<struct BlockLocation> secondaryList);

	/**
	 * Action to reply a request from CLIENT for online CHUNKSERVER list
	 * @param requestId Request ID
	 * @param sockfd Socket ID between the connection
	 * @param chunkserverList List of online chunkserver 
	 */
	void replyChunkserverList(uint32_t requestId, uint32_t sockfd, 
		vector<struct OnlineChunkserver>& chunkserverList);

	/**
	 * Action to reply a request from CLIENT for online CHUNKSERVER list
	 * @param requestId Request ID
	 * @param sockfd Socket ID between the connection
	 * @param chunkserverStatus boolean list of chunkserver status
	 */
	void replyGetChunkserverStatus(uint32_t requestId, uint32_t sockfd,
		vector<bool>& chunkserverStatusRef);

	/**
	 * Action to send current online CHUNKSERVERs to the newly start one
	 * @param newChunkserverSockfd Socket ID of the newly start CHUNKSERVER
	 * @param onlineChunkserverList List of online CHUNKSERVERs with their ip,port,id 
	 */
	void sendOnlineChunkserverList(uint32_t newChunkserverSockfd, 
		vector<struct OnlineChunkserver>& onlineChunkserverList);

private:

};

#endif
