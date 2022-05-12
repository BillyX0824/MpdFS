#include <iostream>
using namespace std;
#include "getchunkserverlistreply.hh"
#include "getchunkserverlistrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

GetChunkserverListReplyMsg::GetChunkserverListReplyMsg(Communicator* communicator) :
		Message(communicator), _chunkserverListRef(_chunkserverList) {

}

GetChunkserverListReplyMsg::GetChunkserverListReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd, vector<struct OnlineChunkserver>& chunkserverListRef) :
		Message(communicator), _chunkserverListRef(chunkserverListRef) {
	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
}

void GetChunkserverListReplyMsg::prepareProtocolMsg() {
	string serializedString;


	ncvfs::GetChunkserverListReplyPro getChunkserverListReplyPro;

	vector<struct OnlineChunkserver>::iterator it;

	for (it = _chunkserverListRef.begin(); it < _chunkserverListRef.end(); ++it) {
		ncvfs::OnlineChunkserverPro* onlineChunkserverPro = 
			getChunkserverListReplyPro.add_onlinechunkserverlist();
		onlineChunkserverPro->set_chunkserverid((*it).chunkserverId);
		onlineChunkserverPro->set_chunkserverip((*it).chunkserverIp);
		onlineChunkserverPro->set_chunkserverport((*it).chunkserverPort);
	}

	if (!getChunkserverListReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_CHUNKSERVER_LIST_REPLY);
	setProtocolMsg(serializedString);

}

void GetChunkserverListReplyMsg::parse(char* buf) {


	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetChunkserverListReplyPro getChunkserverListReplyPro;
	getChunkserverListReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_chunkserverList.clear();
	for (int i = 0; i < getChunkserverListReplyPro.onlinechunkserverlist_size(); ++i) {
		struct OnlineChunkserver tmpOnlineChunkserver;
		tmpOnlineChunkserver.chunkserverId = getChunkserverListReplyPro.onlinechunkserverlist(i).chunkserverid();
		tmpOnlineChunkserver.chunkserverIp = getChunkserverListReplyPro.onlinechunkserverlist(i).chunkserverip();
		tmpOnlineChunkserver.chunkserverPort = getChunkserverListReplyPro.onlinechunkserverlist(i).chunkserverport();
		_chunkserverList.push_back(tmpOnlineChunkserver);
	}
	return;

}

void GetChunkserverListReplyMsg::doHandle() {

	GetChunkserverListRequestMsg* getChunkserverListRequestMsg =
			(GetChunkserverListRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getChunkserverListRequestMsg->setChunkserverList(getChunkserverListRequestMsg->getChunkserverList(), _chunkserverList);
	getChunkserverListRequestMsg->setStatus(READY);

}

void GetChunkserverListReplyMsg::printProtocol() {
	debug("%s\n", "[GET_CHUNKSERVER_LIST_REPLY] GOT.");
	for (uint32_t i = 0; i < (uint32_t) _chunkserverList.size(); i++) {
		debug("LIST i=%" PRIu32 " ip = %" PRIu32 " port = %" PRIu32 "\n",i, _chunkserverList[i].chunkserverIp, _chunkserverList[i].chunkserverPort);
	}
}
