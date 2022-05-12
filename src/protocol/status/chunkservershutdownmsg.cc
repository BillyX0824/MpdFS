#include "chunkservershutdownmsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../monitor/monitor.hh"
extern Monitor* monitor;
#endif

ChunkserverShutdownMsg::ChunkserverShutdownMsg(Communicator* communicator) :
		Message(communicator) {

}

ChunkserverShutdownMsg::ChunkserverShutdownMsg(Communicator* communicator, uint32_t chunkserverSockfd,
		uint32_t chunkserverId) :
		Message(communicator) {

	_sockfd = chunkserverSockfd;
	_chunkserverId = chunkserverId;
}

void ChunkserverShutdownMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ChunkserverShutdownPro chunkserverShutdownPro;
	chunkserverShutdownPro.set_chunkserverid(_chunkserverId);

	if (!chunkserverShutdownPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (CHUNKSERVER_SHUTDOWN);
	setProtocolMsg(serializedString);

}

void ChunkserverShutdownMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::ChunkserverShutdownPro chunkserverShutdownPro;
	chunkserverShutdownPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_chunkserverId = chunkserverShutdownPro.chunkserverid();

}

void ChunkserverShutdownMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->ChunkserverShutdownProcessor(_msgHeader.requestId, _sockfd, _chunkserverId);
#endif
}

void ChunkserverShutdownMsg::printProtocol() {
	debug("[CHUNKSERVER_SHUTDOWN] Chunkserver ID = %" PRIu32 "\n", _chunkserverId);
}
