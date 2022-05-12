#include <iostream>
#include "getsegmentinforequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MASTER
#include "../../master/master.hh"
extern Master* master;
#endif

using namespace std;

GetSegmentInfoRequestMsg::GetSegmentInfoRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetSegmentInfoRequestMsg::GetSegmentInfoRequestMsg(Communicator* communicator,
		uint32_t dstSockfd, uint64_t segmentId, uint32_t chunkserverId, bool needReply, bool isRecovery) :
		Message(communicator) {

	_sockfd = dstSockfd;
	_segmentId = segmentId;
	_chunkserverId = chunkserverId;
	_needReply = needReply;
	_isRecovery = isRecovery;
}

void GetSegmentInfoRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetSegmentInfoRequestPro getSegmentInfoRequestPro;
	getSegmentInfoRequestPro.set_segmentid(_segmentId);
	getSegmentInfoRequestPro.set_chunkserverid(_chunkserverId);
	getSegmentInfoRequestPro.set_needreply(_needReply);
	getSegmentInfoRequestPro.set_isrecovery(_isRecovery);

	if (!getSegmentInfoRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_SEGMENT_INFO_REQUEST);
	setProtocolMsg(serializedString);

}

void GetSegmentInfoRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSegmentInfoRequestPro getSegmentInfoRequestPro;
	getSegmentInfoRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = getSegmentInfoRequestPro.segmentid();
	_chunkserverId = getSegmentInfoRequestPro.chunkserverid();
	_needReply = getSegmentInfoRequestPro.needreply();
	_isRecovery = getSegmentInfoRequestPro.isrecovery();

}

void GetSegmentInfoRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MASTER
	master->getSegmentInfoProcessor (_msgHeader.requestId, _sockfd, _segmentId, _chunkserverId, _needReply, _isRecovery);
#endif
}

void GetSegmentInfoRequestMsg::printProtocol() {
	debug("[GET_SEGMENT_INFO_REQUEST] Segment ID = %" PRIu64 ", isRecovery = %d\n", _segmentId, _isRecovery);
}

vector<uint32_t> GetSegmentInfoRequestMsg::getNodeList() {
	return _nodeList;
}

void GetSegmentInfoRequestMsg::setNodeList(vector<uint32_t> nodeList) {
	_nodeList = nodeList;
}

CodingScheme GetSegmentInfoRequestMsg::getCodingScheme() {
	return _codingScheme;
}

void GetSegmentInfoRequestMsg::setCodingScheme(CodingScheme codingScheme) {
	_codingScheme = codingScheme;
}

string GetSegmentInfoRequestMsg::getCodingSetting() {
	return _codingSetting;
}

void GetSegmentInfoRequestMsg::setCodingSetting(string codingSetting) {
	_codingSetting = codingSetting;
}

uint64_t GetSegmentInfoRequestMsg::getSegmentSize() {
	return _segmentSize;
}

void GetSegmentInfoRequestMsg::setSegmentSize(uint32_t segmentSize) {
	_segmentSize = segmentSize;
}

