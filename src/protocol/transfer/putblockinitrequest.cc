#include "putblockinitrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"

#ifdef COMPILE_FOR_CHUNKSERVER
#include "../../chunkserver/chunkserver.hh"
extern Chunkserver* chunkserver;
#endif

PutBlockInitRequestMsg::PutBlockInitRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

PutBlockInitRequestMsg::PutBlockInitRequestMsg(Communicator* communicator,
		uint32_t chunkserverSockfd, uint64_t segmentId, uint32_t blockId,
		uint32_t blockSize, uint32_t chunkCount,uint32_t blockIdOfDelta,uint32_t deltaId,DataMsgType dataMsgType,
		string updateKey) :
		Message(communicator) {

	_sockfd = chunkserverSockfd;
	_segmentId = segmentId;
	_blockId = blockId;
	_blockSize = blockSize;
	_chunkCount = chunkCount;
	_dataMsgType = dataMsgType;
	_updateKey = updateKey;
	_blockIdOfDelta = blockIdOfDelta;
	_deltaId = deltaId;
}

void PutBlockInitRequestMsg::prepareProtocolMsg() {
	string serializedString;
	ncvfs::PutBlockInitRequestPro putBlockInitRequestPro;
	putBlockInitRequestPro.set_segmentid(_segmentId);
	putBlockInitRequestPro.set_blockid(_blockId);
	putBlockInitRequestPro.set_blocksize(_blockSize);
	putBlockInitRequestPro.set_chunkcount(_chunkCount);
	putBlockInitRequestPro.set_blockidofdelta(_blockIdOfDelta);
	putBlockInitRequestPro.set_deltaid(_deltaId);
	putBlockInitRequestPro.set_datamsgtype((ncvfs::DataMsgPro_DataMsgType)_dataMsgType);
	putBlockInitRequestPro.set_updatekey(_updateKey);

	if (!putBlockInitRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(PUT_BLOCK_INIT_REQUEST);
	setProtocolMsg(serializedString);

}

void PutBlockInitRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutBlockInitRequestPro putBlockInitRequestPro;
	putBlockInitRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = putBlockInitRequestPro.segmentid();
	_blockId = putBlockInitRequestPro.blockid();
	_blockSize = putBlockInitRequestPro.blocksize();
	_chunkCount = putBlockInitRequestPro.chunkcount();
	_blockIdOfDelta = putBlockInitRequestPro.blockidofdelta();
	_deltaId = putBlockInitRequestPro.deltaid();
	_dataMsgType = (DataMsgType) putBlockInitRequestPro.datamsgtype();
	_updateKey = putBlockInitRequestPro.updatekey();
}

void PutBlockInitRequestMsg::doHandle() {
#ifdef COMPILE_FOR_CHUNKSERVER
	debug(
			"[PUT_BLOCK_INIT] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 ", Length = %" PRIu32 ", Count = %" PRIu32 ", dataMsgType = %d\n",
			_segmentId, _blockId, _blockSize, _chunkCount, _dataMsgType);
	chunkserver->putBlockInitProcessor(_msgHeader.requestId, _sockfd, _segmentId,
			_blockId, _blockSize, _chunkCount,_blockIdOfDelta,_deltaId, _dataMsgType, _updateKey);
#endif
}

void PutBlockInitRequestMsg::printProtocol() {
	debug(
			"[PUT_BLOCK_INIT] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 ", Length = %" PRIu32 ", Count = %" PRIu32 ", blockIdOfDelta = %" PRIu32 ", dataMsgType = %d\n",
			_segmentId, _blockId, _blockSize, _chunkCount,_blockIdOfDelta, _dataMsgType);
}
