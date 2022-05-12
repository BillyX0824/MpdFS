#ifndef __CHUNKSERVERSTAT_HH__
#define __CHUNKSERVERSTAT_HH__

#include <thread>
#include <mutex>
using namespace std;

extern mutex chunkserverStatMapMutex;

enum ChunkserverHealthStat{
	ONLINE, OFFLINE, RECOVERING
};

struct ChunkserverStat {
	ChunkserverStat() { }
	ChunkserverStat(uint32_t id, uint32_t sockfd, uint32_t cap, 
			uint32_t load, enum ChunkserverHealthStat health, uint32_t ts): chunkserverId(id), chunkserverSockfd(sockfd),
	chunkserverCapacity(cap), chunkserverLoading(load),	chunkserverHealth(health), timestamp(ts) { } 
	ChunkserverStat(uint32_t id, uint32_t sockfd, uint32_t cap, 
			uint32_t load,  enum ChunkserverHealthStat health, uint32_t ip, uint16_t port,
			uint32_t ts): 
		chunkserverId(id), chunkserverSockfd(sockfd), chunkserverCapacity(cap), chunkserverLoading(load),
		chunkserverHealth(health), chunkserverIp(ip), chunkserverPort(port), timestamp(ts) { } 

	void out() {
		printf("CHUNKSERVER[id=%d,ip=%d,port=%d,sockfd=%d],cap=%d load=%d health =%d\n",chunkserverId, chunkserverIp, chunkserverPort, chunkserverSockfd, chunkserverCapacity, chunkserverLoading, chunkserverHealth);
	}
	uint32_t chunkserverId;
	uint32_t chunkserverSockfd;
	uint32_t chunkserverCapacity;
	uint32_t chunkserverLoading;
	enum ChunkserverHealthStat chunkserverHealth;	// UP DOWN OUT
	uint32_t chunkserverIp;
	uint32_t chunkserverPort;
	uint32_t timestamp;
};

struct ChunkserverLBStat {
	ChunkserverLBStat() {}
	ChunkserverLBStat(uint64_t primaryCount, uint64_t diskCount):
		primaryCount(primaryCount), diskCount(diskCount) {}
	uint64_t primaryCount;
	uint64_t diskCount;
};


#endif
