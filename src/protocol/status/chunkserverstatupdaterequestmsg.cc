#include "chunkserverstatupdaterequestmsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_CHUNKSERVER
#include "../../chunkserver/chunkserver.hh"
extern Chunkserver* chunkserver;
#endif

ChunkserverStatUpdateRequestMsg::ChunkserverStatUpdateRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

ChunkserverStatUpdateRequestMsg::ChunkserverStatUpdateRequestMsg(Communicator* communicator, uint32_t chunkserverSockfd) :
		Message(communicator) {

	_sockfd = chunkserverSockfd;
}

void ChunkserverStatUpdateRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ChunkserverStatUpdateRequestPro chunkserverStatUpdateRequestPro;

	if (!chunkserverStatUpdateRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (CHUNKSERVERSTAT_UPDATE_REQUEST);
	setProtocolMsg(serializedString);

}

void ChunkserverStatUpdateRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::ChunkserverStatUpdateRequestPro chunkserverStatUpdateRequestPro;
	chunkserverStatUpdateRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

}

void ChunkserverStatUpdateRequestMsg::doHandle() {
#ifdef COMPILE_FOR_CHUNKSERVER
	chunkserver->ChunkserverStatUpdateRequestProcessor(_msgHeader.requestId, _sockfd);
#endif
}

void ChunkserverStatUpdateRequestMsg::printProtocol() {
}
