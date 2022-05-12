#ifndef __STATMODULE_HH__
#define __STATMODULE_HH__

#include <stdint.h>
#include <map>
#include "../common/chunkserverstat.hh"
#include "../communicator/communicator.hh"
#include "../protocol/status/chunkserverstatupdaterequestmsg.hh"

using namespace std;

class StatModule {

public:
	/**
	 * Constructor for statmodule
	 * @param mapRef Reference of the map which store all the chunkserver status
	 */
	StatModule(map<uint32_t, struct ChunkserverStat>& mapRef);

	/**
	 * Periodically update the chunkserver stat map by communicate each chunkserver
	 * @param communicator Monitor communicator 
	 * @param updatePeriod The sleep time between update requests
	 */
	void updateChunkserverStatMap (Communicator* communicator, uint32_t updatePeriod);

	/**  
	 * Remove an chunkserver status entry by its chunkserverId 
	 * @param chunkserverId CHUNKSERVER Id
	 */
	void removeStatById (uint32_t chunkserverId);

	/**  
	 * Remove an chunkserver status entry by its chunkserverId 
	 * @param sockfd CHUNKSERVER socket id
	 */
	void removeStatBySockfd (uint32_t sockfd);


	/**
	 * Set an chunkserver status entry, if not in the map, create it, else update the result
	 * @param chunkserverId CHUNKSERVER ID
	 * @param sockfd Sockfd between the monitor and the chunkserver
	 * @param capacity Free space of the CHUNKSERVER
	 * @param loading Current CPU loading of the CHUNKSERVER
	 * @param health Health status of the CHUNKSERVER
	 */
	void setStatById (uint32_t chunkserverId, uint32_t sockfd, uint32_t capacity,
		 uint32_t loading, enum ChunkserverHealthStat health);

	/**
	 * Set an chunkserver status entry, if not in the map, create it, else update the result
	 * @param chunkserverId CHUNKSERVER ID
	 * @param sockfd Sockfd between the monitor and the chunkserver
	 * @param capacity Free space of the CHUNKSERVER
	 * @param loading Current CPU loading of the CHUNKSERVER
	 * @param health Health status of the CHUNKSERVER
	 * @param ip IP of the CHUNKSERVER for other components' connection
	 * @param port Port of the CHUNKSERVER for other components' connetion
	 */
	void setStatById (uint32_t chunkserverId, uint32_t sockfd, uint32_t capacity,
		 uint32_t loading, enum ChunkserverHealthStat health, uint32_t ip, uint16_t port);
	
	/**
	 * Get the current online Chunkserver list to form a list of struct OnlineChunkserver
	 * @param list a vector reference of online chunkserver list with its ip, port, id
	 */
	void getOnlineChunkserverList(vector<struct OnlineChunkserver>& list);

	/**
	 * When a new chunkserver register, broadcast its id,ip,port to all online chunkservers
	 * @param communicator pointer to monitor communicator
	 * @param chunkserverId new CHUNKSERVER ID
	 * @param ip IP of the new CHUNKSERVER for other components' connection
	 * @param port Port of the CHUNKSERVER for other components' connetion
	 */
	void broadcastNewChunkserver(Communicator* communicator, uint32_t chunkserverId, 
		uint32_t ip, uint32_t port);

	/**
	 * When a get chunkserver status request for degraded read
	 * @param chunkserverListRef request reference of chunkserver list
	 * @param chunkserverStatusRef return reference
	 */
	void getChunkserverStatus(vector<uint32_t>& chunkserverListRef, vector<bool>& chunkserverStatusRef);

private:

	/**
	 * Reference of the map defined in the monitor class 
	 */
	map<uint32_t, struct ChunkserverStat>& _chunkserverStatMap;

};

#endif
