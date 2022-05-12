#include "statmodule.hh"
#include "../common/onlinechunkserver.hh"
#include "../common/debug.hh"
#include "../protocol/status/newchunkserverregistermsg.hh"
#include <ctime>


/*  Constructor */
StatModule::StatModule(map<uint32_t, struct ChunkserverStat>& mapRef):
	_chunkserverStatMap(mapRef) { 

}

void StatModule::updateChunkserverStatMap (Communicator* communicator, uint32_t
	updatePeriod) {

	while (1) {
		printf("-----------------10s start----------------\n");
		{
			lock_guard<mutex> lk(chunkserverStatMapMutex);
			for(auto& entry: _chunkserverStatMap) {
				if (entry.second.chunkserverHealth == ONLINE) {

					printf("Entry %3d:\n",entry.first);
					printf("		 "); 
					entry.second.out();			

					ChunkserverStatUpdateRequestMsg* requestMsg = new 
					  ChunkserverStatUpdateRequestMsg(communicator, entry.second.chunkserverSockfd);
					requestMsg -> prepareProtocolMsg();
					// Do not need to wait for reply.
					communicator -> addMessage (requestMsg);
				}
			}
		}
		sleep(updatePeriod);
		printf("-----------------10s finish--------------\n");
	}

}

void StatModule::removeStatById (uint32_t chunkserverId) {
	lock_guard<mutex> lk(chunkserverStatMapMutex);
	_chunkserverStatMap.erase(chunkserverId);
}

void StatModule::removeStatBySockfd (uint32_t sockfd) {
	// Set to OFFLINE
	lock_guard<mutex> lk(chunkserverStatMapMutex);
	map<uint32_t, struct ChunkserverStat>::iterator p;
	p = _chunkserverStatMap.begin();
	while (p != _chunkserverStatMap.end()) {
		if (p->second.chunkserverSockfd == sockfd)
			p->second.chunkserverHealth = OFFLINE;
			//_chunkserverStatMap.erase(p++);
			//else
		p++;
	}	
	debug_yellow("Delete sockfd = %" PRIu32 "\n", sockfd);
}

void StatModule::setStatById (uint32_t chunkserverId, uint32_t sockfd, 
	uint32_t capacity, uint32_t loading, enum ChunkserverHealthStat health) {
	map<uint32_t, struct ChunkserverStat>::iterator iter;

	lock_guard<mutex> lk(chunkserverStatMapMutex);
	iter = _chunkserverStatMap.find(chunkserverId);
	if (iter == _chunkserverStatMap.end()) {
		_chunkserverStatMap[chunkserverId] =ChunkserverStat(chunkserverId, sockfd, capacity, loading, health,
		time(NULL));
	} else {
		iter->second.chunkserverSockfd = sockfd;
		iter->second.chunkserverCapacity = capacity;
		iter->second.chunkserverLoading = loading;
		iter->second.chunkserverHealth = health;
		iter->second.timestamp = time(NULL);
	}
}

void StatModule::setStatById (uint32_t chunkserverId, uint32_t sockfd, 
	uint32_t capacity, uint32_t loading, enum ChunkserverHealthStat health, uint32_t ip,
	uint16_t port) {
	map<uint32_t, struct ChunkserverStat>::iterator iter;

	lock_guard<mutex> lk(chunkserverStatMapMutex);
	iter = _chunkserverStatMap.find(chunkserverId);
	if (iter == _chunkserverStatMap.end()) {
		_chunkserverStatMap[chunkserverId] =ChunkserverStat(chunkserverId, sockfd, capacity, loading, health,
		ip, port, time(NULL));
	} else {
		iter->second.chunkserverSockfd = sockfd;
		iter->second.chunkserverCapacity = capacity;
		iter->second.chunkserverLoading = loading;
		iter->second.chunkserverHealth = health;
		iter->second.chunkserverIp = ip;
		iter->second.chunkserverPort = port;
		iter->second.timestamp = time(NULL);
	}
}



void StatModule::getOnlineChunkserverList(vector<struct OnlineChunkserver>& list) {
	list.clear();
	{
		lock_guard<mutex> lk(chunkserverStatMapMutex);
		for (auto& entry: _chunkserverStatMap) {
			if (entry.second.chunkserverHealth == ONLINE) 
				list.push_back (OnlineChunkserver (entry.first, 
					entry.second.chunkserverIp, entry.second.chunkserverPort));
		}
	}
}

void StatModule::broadcastNewChunkserver(Communicator* communicator,
	uint32_t chunkserverId, uint32_t ip, uint32_t port) {

	lock_guard<mutex> lk(chunkserverStatMapMutex);
	for (auto& entry: _chunkserverStatMap) {
		if (entry.second.chunkserverHealth == ONLINE) {

			NewChunkserverRegisterMsg* newChunkserverRegisterMsg = new NewChunkserverRegisterMsg(
				communicator, entry.second.chunkserverSockfd, chunkserverId, ip, port);
			newChunkserverRegisterMsg->prepareProtocolMsg();

			communicator->addMessage(newChunkserverRegisterMsg);
		}
	}
}


void StatModule::getChunkserverStatus(vector<uint32_t>& chunkserverListRef, 
	vector<bool>& chunkserverStatusRef) {

	chunkserverStatusRef.clear();
	lock_guard<mutex> lk(chunkserverStatMapMutex);
	for (uint32_t id: chunkserverListRef) {
		map<uint32_t, struct ChunkserverStat>::iterator it = _chunkserverStatMap.find(id);
		if (it != _chunkserverStatMap.end()) {
			if (it->second.chunkserverHealth == ONLINE)
				chunkserverStatusRef.push_back(true);
			else 
				chunkserverStatusRef.push_back(false);
		} else {
			chunkserverStatusRef.push_back(false);
		}
	}
}
