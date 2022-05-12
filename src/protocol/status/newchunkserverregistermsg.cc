#include <iostream>
using namespace std;
#include "newchunkserverregistermsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_CHUNKSERVER
#include "../../chunkserver/chunkserver.hh"
extern Chunkserver* chunkserver;
#endif

NewChunkserverRegisterMsg::NewChunkserverRegisterMsg(Communicator* communicator) :
		Message(communicator) {

}

NewChunkserverRegisterMsg::NewChunkserverRegisterMsg(Communicator* communicator, uint32_t sockfd,
		uint32_t chunkserverId, uint32_t chunkserverIp, uint32_t chunkserverPort) :
		Message(communicator) {

	_sockfd = sockfd;
	_chunkserverId = chunkserverId;
	_chunkserverIp = chunkserverIp;
	_chunkserverPort = chunkserverPort;
}

void NewChunkserverRegisterMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::NewChunkserverRegisterPro newChunkserverRegisterPro;
	newChunkserverRegisterPro.set_chunkserverid(_chunkserverId);
	newChunkserverRegisterPro.set_chunkserverip(_chunkserverIp);
	newChunkserverRegisterPro.set_chunkserverport(_chunkserverPort);

	if (!newChunkserverRegisterPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (NEW_CHUNKSERVER_REGISTER);
	setProtocolMsg(serializedString);

}

void NewChunkserverRegisterMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::NewChunkserverRegisterPro newChunkserverRegisterPro;
	newChunkserverRegisterPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_chunkserverId = newChunkserverRegisterPro.chunkserverid();
	_chunkserverIp = newChunkserverRegisterPro.chunkserverip();
	_chunkserverPort = newChunkserverRegisterPro.chunkserverport();

}

void NewChunkserverRegisterMsg::doHandle() {
#ifdef COMPILE_FOR_CHUNKSERVER
	chunkserver->NewChunkserverRegisterProcessor(_msgHeader.requestId, _sockfd, _chunkserverId, 
		_chunkserverIp, _chunkserverPort);
#endif
}

void NewChunkserverRegisterMsg::printProtocol() {
	debug("[NEW_CHUNKSERVER_REGISTER] Chunkserver ID = %" PRIu32 "\n", _chunkserverId);
}
