#include <iostream>
using namespace std;
#include "chunkserverstartupmsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../../monitor/monitor.hh"
extern Monitor* monitor;
#endif

ChunkserverStartupMsg::ChunkserverStartupMsg(Communicator* communicator) :
		Message(communicator) {

}

ChunkserverStartupMsg::ChunkserverStartupMsg(Communicator* communicator, uint32_t chunkserverSockfd,
		uint32_t chunkserverId, uint32_t capacity, uint32_t loading) :
		Message(communicator) {

	_sockfd = chunkserverSockfd;
	_chunkserverId = chunkserverId;
	_capacity = capacity;
	_loading = loading;
}

ChunkserverStartupMsg::ChunkserverStartupMsg(Communicator* communicator, uint32_t chunkserverSockfd,
		uint32_t chunkserverId, uint32_t capacity, uint32_t loading, uint32_t ip,
		uint16_t port) :
		Message(communicator) {

	_sockfd = chunkserverSockfd;
	_chunkserverId = chunkserverId;
	_capacity = capacity;
	_loading = loading;
	_chunkserverIp = ip;
	_chunkserverPort = port;
}

void ChunkserverStartupMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ChunkserverStartupPro chunkserverStartupPro;
	chunkserverStartupPro.set_chunkserverid(_chunkserverId);
	chunkserverStartupPro.set_chunkservercapacity(_capacity);
	chunkserverStartupPro.set_chunkserverloading(_loading);
	chunkserverStartupPro.set_chunkserverip(_chunkserverIp);
	chunkserverStartupPro.set_chunkserverport(_chunkserverPort);

	if (!chunkserverStartupPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (CHUNKSERVER_STARTUP);
	setProtocolMsg(serializedString);

}

void ChunkserverStartupMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::ChunkserverStartupPro chunkserverStartupPro;
	chunkserverStartupPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_chunkserverId = chunkserverStartupPro.chunkserverid();
	_capacity = chunkserverStartupPro.chunkservercapacity();
	_loading = chunkserverStartupPro.chunkserverloading();
	_chunkserverIp = chunkserverStartupPro.chunkserverip();
	_chunkserverPort = chunkserverStartupPro.chunkserverport();

}

void ChunkserverStartupMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->ChunkserverStartupProcessor(_msgHeader.requestId, _sockfd, _chunkserverId, 
		_capacity, _loading, _chunkserverIp, _chunkserverPort);
#endif
}

void ChunkserverStartupMsg::printProtocol() {
	debug("[CHUNKSERVER_STARTUP] Chunkserver ID = %" PRIu32 ", capacity = %" PRIu32 ", loading = %" PRIu32 "\n",
			_chunkserverId, _capacity, _loading);
}
