#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "getchunkserverstatusrequestmsg.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../../monitor/monitor.hh"
extern Monitor* monitor;
#endif

GetChunkserverStatusRequestMsg::GetChunkserverStatusRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetChunkserverStatusRequestMsg::GetChunkserverStatusRequestMsg(Communicator* communicator,
		uint32_t sockfd, vector<uint32_t>& chunkserverListRef) :
		Message(communicator) {

	_sockfd = sockfd;
	_chunkserverList = chunkserverListRef;
}

void GetChunkserverStatusRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetChunkserverStatusRequestPro getChunkserverStatusRequestPro;
	for (uint32_t chunkserverid : _chunkserverList) {
		getChunkserverStatusRequestPro.add_chunkserverids(chunkserverid);
	}

	if (!getChunkserverStatusRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_CHUNKSERVER_STATUS_REQUEST);
	setProtocolMsg(serializedString);

}

void GetChunkserverStatusRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetChunkserverStatusRequestPro getChunkserverStatusRequestPro;
	getChunkserverStatusRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	for (int i = 0; i < getChunkserverStatusRequestPro.chunkserverids_size(); ++i) {
		_chunkserverList.push_back(getChunkserverStatusRequestPro.chunkserverids(i));
	}

}

void GetChunkserverStatusRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->getChunkserverStatusRequestProcessor (_msgHeader.requestId, _sockfd, _chunkserverList);
#endif
}

void GetChunkserverStatusRequestMsg::printProtocol() {
	debug("%s\n", "[GET_CHUNKSERVER_STATUS_REQUEST]");
}

void GetChunkserverStatusRequestMsg::setChunkserverStatus(vector<bool>& chunkserverStatusRef) {
	_chunkserverStatus = chunkserverStatusRef;
	return;
}

vector<bool>& GetChunkserverStatusRequestMsg::getChunkserverStatus() {
	return _chunkserverStatus;
}
