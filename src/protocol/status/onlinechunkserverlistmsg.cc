#include <iostream>
using namespace std;
#include "onlinechunkserverlistmsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_CHUNKSERVER
#include "../../chunkserver/chunkserver.hh"
extern Chunkserver* chunkserver;
#endif

OnlineChunkserverListMsg::OnlineChunkserverListMsg(Communicator* communicator) :
		Message(communicator), _onlineChunkserverListRef(_onlineChunkserverList) {

}

OnlineChunkserverListMsg::OnlineChunkserverListMsg(Communicator* communicator, uint32_t sockfd,
		vector<struct OnlineChunkserver>& onlineChunkserverListRef) :
		Message(communicator), _onlineChunkserverListRef(onlineChunkserverListRef) {

	_sockfd = sockfd;
}

void OnlineChunkserverListMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::OnlineChunkserverListPro onlineChunkserverListPro;

	vector<struct OnlineChunkserver>::iterator it;

	for (it = _onlineChunkserverListRef.begin(); it < _onlineChunkserverListRef.end(); ++it) {
		ncvfs::OnlineChunkserverPro* onlineChunkserverPro =
				onlineChunkserverListPro.add_onlinechunkserverlist();
		onlineChunkserverPro->set_chunkserverid((*it).chunkserverId);
		onlineChunkserverPro->set_chunkserverip((*it).chunkserverIp);
		onlineChunkserverPro->set_chunkserverport((*it).chunkserverPort);
	}

	if (!onlineChunkserverListPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(ONLINE_CHUNKSERVER_LIST);
	setProtocolMsg(serializedString);

}

void OnlineChunkserverListMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::OnlineChunkserverListPro onlineChunkserverListPro;
	onlineChunkserverListPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_onlineChunkserverList.clear();
	for (int i = 0; i < onlineChunkserverListPro.onlinechunkserverlist_size(); ++i) {
		struct OnlineChunkserver tmpOnlineChunkserver;
		tmpOnlineChunkserver.chunkserverId = onlineChunkserverListPro.onlinechunkserverlist(i).chunkserverid();
		tmpOnlineChunkserver.chunkserverIp = onlineChunkserverListPro.onlinechunkserverlist(i).chunkserverip();
		tmpOnlineChunkserver.chunkserverPort = onlineChunkserverListPro.onlinechunkserverlist(i).chunkserverport();
		_onlineChunkserverList.push_back(tmpOnlineChunkserver);
	}

}

void OnlineChunkserverListMsg::doHandle() {
#ifdef COMPILE_FOR_CHUNKSERVER
	chunkserver->OnlineChunkserverListProcessor(_msgHeader.requestId, _sockfd, _onlineChunkserverList);
#endif
}

void OnlineChunkserverListMsg::printProtocol() {
	debug("%s\n", "[ONLINE_CHUNKSERVER_LIST] built");
	for (uint32_t i = 0; i < _onlineChunkserverList.size(); i++) {
		debug(
				"[ONLINE_CHUNKSERVER_LIST] contains %" PRIu32 "  = (%" PRIu32 " %" PRIu32 " %" PRIu32 ")\n",
				i, _onlineChunkserverList[i].chunkserverId, _onlineChunkserverList[i].chunkserverIp, _onlineChunkserverList[i].chunkserverPort);
	}
}
