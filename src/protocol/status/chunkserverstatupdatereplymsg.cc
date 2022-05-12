#include "chunkserverstatupdatereplymsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../monitor/monitor.hh"
extern Monitor* monitor;
#endif

ChunkserverStatUpdateReplyMsg::ChunkserverStatUpdateReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

ChunkserverStatUpdateReplyMsg::ChunkserverStatUpdateReplyMsg(Communicator* communicator, uint32_t chunkserverSockfd,
		uint32_t chunkserverId, uint32_t capacity, uint32_t loading) :
		Message(communicator) {

	_sockfd = chunkserverSockfd;
	_chunkserverId = chunkserverId;
	_capacity = capacity;
	_loading = loading;
	
}

void ChunkserverStatUpdateReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ChunkserverStatUpdateReplyPro chunkserverStatUpdateReplyPro;
	chunkserverStatUpdateReplyPro.set_chunkserverid(_chunkserverId);
	chunkserverStatUpdateReplyPro.set_chunkservercapacity(_capacity);
	chunkserverStatUpdateReplyPro.set_chunkserverloading(_loading);

	if (!chunkserverStatUpdateReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (CHUNKSERVERSTAT_UPDATE_REPLY);
	setProtocolMsg(serializedString);

}

void ChunkserverStatUpdateReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::ChunkserverStatUpdateReplyPro chunkserverStatUpdateReplyPro;
	chunkserverStatUpdateReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_chunkserverId = chunkserverStatUpdateReplyPro.chunkserverid();
	_capacity = chunkserverStatUpdateReplyPro.chunkservercapacity();
	_loading = chunkserverStatUpdateReplyPro.chunkserverloading();

}

void ChunkserverStatUpdateReplyMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->ChunkserverStatUpdateReplyProcessor(_msgHeader.requestId, _sockfd, _chunkserverId, _capacity, _loading);
#endif
}

void ChunkserverStatUpdateReplyMsg::printProtocol() {
	debug("[CHUNKSERVERSTAT_UPDATE_REPLY] Chunkserver ID = %" PRIu32 ", capacity = %" PRIu64 ", loading = %" PRIu32 "\n",
			_chunkserverId, _capacity, _loading);
}
