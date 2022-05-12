#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "recoverytriggerrequest.hh"

#ifdef COMPILE_FOR_MASTER
#include "../../master/master.hh"
extern Master* master;
#endif


RecoveryTriggerRequestMsg::RecoveryTriggerRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

RecoveryTriggerRequestMsg::RecoveryTriggerRequestMsg(Communicator* communicator,
		uint32_t masterSockfd, const vector<uint32_t> &chunkserverList, bool dstSpecified,
		const vector<uint32_t> &dstSpec) :
		Message(communicator) {

	_sockfd = masterSockfd;
	_chunkserverList = chunkserverList;
	_dstSpecified = dstSpecified;
	_dstSpecChunkserverList = dstSpec;
}

void RecoveryTriggerRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::RecoveryTriggerRequestPro recoveryTriggerRequestPro;
	for (uint32_t chunkserver : _chunkserverList) {
		recoveryTriggerRequestPro.add_chunkserverlist(chunkserver);
	}
	recoveryTriggerRequestPro.set_dstspecified(_dstSpecified);
	for (uint32_t chunkserver : _dstSpecChunkserverList) {
		recoveryTriggerRequestPro.add_dstchunkserverlist(chunkserver);
	}

	if (!recoveryTriggerRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (RECOVERY_TRIGGER_REQUEST);
	setProtocolMsg(serializedString);

}

void RecoveryTriggerRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::RecoveryTriggerRequestPro recoveryTriggerRequestPro;
	recoveryTriggerRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	for (int i = 0; i < recoveryTriggerRequestPro.chunkserverlist_size(); ++i) {
			_chunkserverList.push_back(recoveryTriggerRequestPro.chunkserverlist(i));
	}
	for (int i = 0; i < recoveryTriggerRequestPro.dstchunkserverlist_size(); ++i) {
			_dstSpecChunkserverList.push_back(recoveryTriggerRequestPro.dstchunkserverlist(i));
	}
	_dstSpecified = recoveryTriggerRequestPro.dstspecified();
	return;

}

void RecoveryTriggerRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MASTER
	master->recoveryTriggerProcessor (_msgHeader.requestId, _sockfd, _chunkserverList, _dstSpecified, _dstSpecChunkserverList);
#endif
}

void RecoveryTriggerRequestMsg::setSegmentLocations(
	vector<struct SegmentLocation> objLochunkserver) {
	_segmentLocations = objLochunkserver;
}

vector<struct SegmentLocation> RecoveryTriggerRequestMsg::getSegmentLocations() {
	return _segmentLocations;
}

void RecoveryTriggerRequestMsg::printProtocol() {
	debug("%s\n", "GOT MESSAGE [RECOVERY_TRIGGER_REQUEST]");

}

