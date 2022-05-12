#include <iostream>
using namespace std;
#include "getchunkserverstatusreplymsg.hh"
#include "getchunkserverstatusrequestmsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

GetChunkserverStatusReplyMsg::GetChunkserverStatusReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetChunkserverStatusReplyMsg::GetChunkserverStatusReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd, const vector<bool>
		&chunkserverStatusRef) :
		Message(communicator) {
	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_chunkserverStatus = chunkserverStatusRef;
	
}

void GetChunkserverStatusReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetChunkserverStatusReplyPro getChunkserverStatusReplyPro;

	for (bool stat : _chunkserverStatus) {
		getChunkserverStatusReplyPro.add_chunkserverstatus(stat);
	}

	if (!getChunkserverStatusReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_CHUNKSERVER_STATUS_REPLY);
	setProtocolMsg(serializedString);

}

void GetChunkserverStatusReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetChunkserverStatusReplyPro getChunkserverStatusReplyPro;
	getChunkserverStatusReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	for (int i = 0; i < getChunkserverStatusReplyPro.chunkserverstatus_size(); ++i) {
		_chunkserverStatus.push_back(getChunkserverStatusReplyPro.chunkserverstatus(i));
	}
	return;
}

void GetChunkserverStatusReplyMsg::doHandle() {
	GetChunkserverStatusRequestMsg* getChunkserverStatusRequestMsg =
			(GetChunkserverStatusRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getChunkserverStatusRequestMsg->setChunkserverStatus(_chunkserverStatus);
	getChunkserverStatusRequestMsg->setStatus(READY);
}

void GetChunkserverStatusReplyMsg::printProtocol() {
	debug("%s\n", "[GET_CHUNKSERVER_STATUS_REPLY] GOT.");
}
