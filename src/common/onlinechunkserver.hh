#ifndef ONLINECHUNKSERVER_HH_
#define ONLINECHUNKSERVER_HH_

struct OnlineChunkserver {
	OnlineChunkserver() { }
	OnlineChunkserver(uint32_t id, uint32_t ip, uint32_t port):
		chunkserverId(id), chunkserverIp(ip), chunkserverPort(port) { }
	uint32_t chunkserverId;
	uint32_t chunkserverIp;
	uint32_t chunkserverPort;
};

#endif /* ONLINECHUNKSERVER_HH_ */
