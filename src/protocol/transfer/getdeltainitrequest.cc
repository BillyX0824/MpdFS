#include "getdeltainitrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"

#ifdef COMPILE_FOR_CHUNKSERVER
#include "../../chunkserver/chunkserver.hh"
extern Chunkserver* chunkserver;
#endif

GetDeltaInitRequestMsg::GetDeltaInitRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetDeltaInitRequestMsg::GetDeltaInitRequestMsg(Communicator* communicator,
		uint32_t chunkserverSockfd, uint64_t segmentId, uint32_t blockId,
		uint32_t beginDeltaId,uint32_t endDeltaId,DataMsgType dataMsgType) :
		Message(communicator) {
	_sockfd = chunkserverSockfd;
	_segmentId = segmentId;
	_blockId = blockId;
	_dataMsgType = dataMsgType;
	_beginDeltaId = beginDeltaId;
	_endDeltaId = endDeltaId;
	//_getNum = getNum;
}

void GetDeltaInitRequestMsg::prepareProtocolMsg() {
	string serializedString;
	ncvfs::GetDeltaInitRequestPro getDeltaInitRequestMsg;
	getDeltaInitRequestMsg.set_segmentid(_segmentId);
	getDeltaInitRequestMsg.set_blockid(_blockId);
    getDeltaInitRequestMsg.set_begindeltaid(_beginDeltaId);
    getDeltaInitRequestMsg.set_enddeltaid(_endDeltaId);
	//getDeltaInitRequestMsg.set_getNum(_getNum);
	getDeltaInitRequestMsg.set_datamsgtype((ncvfs::DataMsgPro_DataMsgType)_dataMsgType);

	if (!getDeltaInitRequestMsg.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_DELTA_INIT_REQUEST);
	setProtocolMsg(serializedString);

}

void GetDeltaInitRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetDeltaInitRequestPro getDeltaInitRequestPro;
    getDeltaInitRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = getDeltaInitRequestPro.segmentid();
	_blockId = getDeltaInitRequestPro.blockid();
	_beginDeltaId = getDeltaInitRequestPro.begindeltaid();
	_endDeltaId = getDeltaInitRequestPro.enddeltaid();
	//_getNum = getDeltaInitRequestPro.getNum();
	_dataMsgType = (DataMsgType)getDeltaInitRequestPro.datamsgtype();
}

void GetDeltaInitRequestMsg::doHandle() {
#ifdef COMPILE_FOR_CHUNKSERVER
	chunkserver->getDeltaRequestProcessor (_msgHeader.requestId, _sockfd, _segmentId, _blockId, _beginDeltaId, _endDeltaId, _dataMsgType);
#endif
}

void GetDeltaInitRequestMsg::printProtocol() {
	debug_red(
			"[GET_DELTA_INIT_REQUEST] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 ", dataMsgType = %d\n",
			_segmentId, _blockId, _dataMsgType);
}