#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "getchunkserverlistrequest.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../../monitor/monitor.hh"
extern Monitor* monitor;
#endif

GetChunkserverListRequestMsg::GetChunkserverListRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetChunkserverListRequestMsg::GetChunkserverListRequestMsg(Communicator* communicator,
		uint32_t sockfd) :
		Message(communicator) {

	_sockfd = sockfd;
}

void GetChunkserverListRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetChunkserverListRequestPro getChunkserverListRequestPro;

	if (!getChunkserverListRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_CHUNKSERVER_LIST_REQUEST);
	setProtocolMsg(serializedString);

}

void GetChunkserverListRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetChunkserverListRequestPro getChunkserverListRequestPro;
	getChunkserverListRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

}

void GetChunkserverListRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->getChunkserverListProcessor (_msgHeader.requestId, _sockfd);
#endif
}

void GetChunkserverListRequestMsg::printProtocol() {
	debug("%s\n", "[GET_CHUNKSERVER_LIST_REQUEST]");
}

void GetChunkserverListRequestMsg::setChunkserverList(vector<struct OnlineChunkserver>& _chunkserverList,
	vector<struct OnlineChunkserver>& chunkserverList) {
	_chunkserverList = chunkserverList;
	return;
}

vector<struct OnlineChunkserver>& GetChunkserverListRequestMsg::getChunkserverList() {
	return _chunkserverList;
}
