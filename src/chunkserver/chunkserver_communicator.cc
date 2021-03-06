/**
 * chunkserver_communicator.cc
 */

#include <iostream>
#include <cstdio>
#include "chunkserver.hh"
#include "chunkserver_communicator.hh"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../common/debug.hh"
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../common/metadata.hh"
#include "../protocol/metadata/uploadsegmentack.hh"
#include "../protocol/metadata/listdirectoryrequest.hh"
#include "../protocol/metadata/getsegmentinforequest.hh"
#include "../protocol/transfer/putsegmentinitreply.hh"
#include "../protocol/transfer/getblockinitrequest.hh"
#include "../protocol/transfer/getdeltainitrequest.hh"
#include "../protocol/transfer/putblockinitrequest.hh"
#include "../protocol/transfer/putblockinitreply.hh"
#include "../protocol/transfer/segmenttransferendreply.hh"
#include "../protocol/transfer/blocktransferendrequest.hh"
#include "../protocol/transfer/blocktransferendreply.hh"
#include "../protocol/transfer/blockdatamsg.hh"
#include "../protocol/nodelist/getsecondarylistrequest.hh"
#include "../protocol/status/chunkserverstartupmsg.hh"
#include "../protocol/status/getchunkserverstatusrequestmsg.hh"
#include "../protocol/status/repairsegmentinfomsg.hh"

using namespace std;

extern Chunkserver* chunkserver;

/**
 * Constructor
 */

ChunkserverCommunicator::ChunkserverCommunicator() {

}

/**
 * Destructor
 */

ChunkserverCommunicator::~ChunkserverCommunicator() {

}

void ChunkserverCommunicator::replyPutSegmentInit(uint32_t requestId,
		uint32_t connectionId, uint64_t segmentId, DataMsgType dataMsgType) {

	PutSegmentInitReplyMsg* putSegmentInitReplyMsg = new PutSegmentInitReplyMsg(
			this, requestId, connectionId, segmentId, dataMsgType);
	putSegmentInitReplyMsg->prepareProtocolMsg();

	addMessage(putSegmentInitReplyMsg);
}

void ChunkserverCommunicator::replyPutBlockInit(uint32_t requestId,
		uint32_t connectionId, uint64_t segmentId, uint32_t blockId) {

	PutBlockInitReplyMsg* putBlockInitReplyMsg = new PutBlockInitReplyMsg(this,
			requestId, connectionId, segmentId, blockId);
	putBlockInitReplyMsg->prepareProtocolMsg();

	addMessage(putBlockInitReplyMsg);
}

void ChunkserverCommunicator::replyPutSegmentEnd(uint32_t requestId,
		uint32_t connectionId, uint64_t segmentId, bool isSmallSegment) {

	SegmentTransferEndReplyMsg* putSegmentEndReplyMsg =
			new SegmentTransferEndReplyMsg(this, requestId, connectionId,
					segmentId, isSmallSegment);
	putSegmentEndReplyMsg->prepareProtocolMsg();

	addMessage(putSegmentEndReplyMsg);
}

void ChunkserverCommunicator::replyPutBlockEnd(uint32_t requestId,
		uint32_t connectionId, uint64_t segmentId, uint32_t blockId,
		uint32_t waitOnRequestId) {

	uint32_t msgRequestId = 0;
	if (waitOnRequestId == 0) {
		msgRequestId = requestId;
	} else {
		msgRequestId = waitOnRequestId;
	}

	BlockTransferEndReplyMsg* blockTransferEndReplyMsg =
			new BlockTransferEndReplyMsg(this, msgRequestId, connectionId,
					segmentId, blockId);
	blockTransferEndReplyMsg->prepareProtocolMsg();

	addMessage(blockTransferEndReplyMsg);

}

uint32_t ChunkserverCommunicator::reportChunkserverFailure(uint32_t chunkserverId) {
	return 0;
}

uint32_t ChunkserverCommunicator::sendBlock(uint32_t sockfd, struct BlockData blockData,
		DataMsgType dataMsgType, string updateKey) {
	uint32_t blockIdOfDelta = blockData.info.blockIdOfDelta;
	uint64_t segmentId = blockData.info.segmentId;
	uint32_t blockId = blockData.info.blockId;
	uint32_t deltaId = blockData.info.deltaId;
    // this is buffer length held in the BlockData, not the block size
	uint32_t length = blockData.info.blockSize;
	char* buf = blockData.buf;
	uint32_t chunkCount = ((length - 1) / _chunkSize) + 1;
	vector<offset_length_t> offsetLength = blockData.info.offlenVector;
	vector<BlockLocation> parityList = blockData.info.parityVector;
	
    if (blockIdOfDelta == 999){
        debug_red("Put NONE DELTA from segmentId = %" PRIu64" , blockId = %" PRIu32"\n", segmentId,blockId);
		putBlockInit(sockfd, segmentId, blockId, length, chunkCount, blockIdOfDelta, deltaId, dataMsgType, updateKey);
    }else {
		
	// step 1: send init message, wait for ack
	
    debug("XXXXX segmentId = %" PRIu64 " blockid = %" PRIu32 " blocksize = %" PRIu32 "\n", segmentId, blockId, length);
	debug("Put Block Init to FD = %" PRIu32 "\n", sockfd);
	putBlockInit(sockfd, segmentId, blockId, length, chunkCount, blockIdOfDelta, deltaId, dataMsgType, updateKey);
	debug("Put Block Init ACK-ed from FD = %" PRIu32 "\n", sockfd);

	// step 2: send data
	
	uint64_t byteToSend = 0;
	uint64_t byteProcessed = 0;
	uint64_t byteRemaining = length;
	uint32_t ifrom = blockData.info.ifrom;
	if(dataMsgType == DELTA){
	    updateKey = to_string(deltaId);
	}
	if(dataMsgType == UPLOAD){
		updateKey = to_string(ifrom);
	}
	while (byteProcessed < length) {

		if (byteRemaining > _chunkSize) {
			byteToSend = _chunkSize;
		} else {
			byteToSend = byteRemaining;
		}
		if(dataMsgType == DELTA){
        debug_red("send off=%" PRIu64 ", size = %" PRIu64 "\n", byteProcessed,byteToSend);
		}
		putBlockData(sockfd, segmentId, blockId, buf, byteProcessed, byteToSend,
				dataMsgType, updateKey);
		byteProcessed += byteToSend;
		byteRemaining -= byteToSend;
	}
	
	// Step 3: Send End message
	
    putBlockEnd(sockfd, segmentId, blockId, dataMsgType, updateKey,
            offsetLength, parityList, blockData.info.codingScheme,
            blockData.info.codingSetting, blockData.info.segmentSize);

	cout << "Put Block ID = " << segmentId << "." << blockId << " Finished"<< endl;
    }
	return 0;
}


void ChunkserverCommunicator::getBlockRequest(uint32_t chunkserverId, uint64_t segmentId,
		uint32_t blockId, vector<offset_length_t> symbols, DataMsgType dataMsgType, bool isParity) {

	uint32_t dstSockfd = getSockfdFromId(chunkserverId);
	GetBlockInitRequestMsg* getBlockInitRequestMsg = new GetBlockInitRequestMsg(
			this, dstSockfd, segmentId, blockId, symbols, dataMsgType, isParity);
	getBlockInitRequestMsg->prepareProtocolMsg();

	addMessage(getBlockInitRequestMsg, false);

}
void ChunkserverCommunicator::getDeltaRequest(uint32_t chunkserverId, uint64_t segmentId,
									  uint32_t blockId, uint32_t beginDeltaId,uint32_t endDeltaId,DataMsgType dataMsgType) {
	uint32_t dstSockfd = getSockfdFromId(chunkserverId);
	GetDeltaInitRequestMsg* getDeltaInitRequestMsg = new GetDeltaInitRequestMsg(
			this, dstSockfd, segmentId, blockId, beginDeltaId,endDeltaId, dataMsgType);
	getDeltaInitRequestMsg->prepareProtocolMsg();

	addMessage(getDeltaInitRequestMsg, false);

}

vector<struct BlockLocation> ChunkserverCommunicator::getChunkserverListRequest(
		uint64_t segmentId, ComponentType dstComponent, uint32_t blockCount,
		uint32_t primaryId, uint64_t blockSize) {

	GetSecondaryListRequestMsg* getSecondaryListRequestMsg =
			new GetSecondaryListRequestMsg(this, getMonitorSockfd(), blockCount,
					primaryId, blockSize);
	getSecondaryListRequestMsg->prepareProtocolMsg();

	addMessage(getSecondaryListRequestMsg, true);
	MessageStatus status = getSecondaryListRequestMsg->waitForStatusChange();

	if (status == READY) {
		vector<struct BlockLocation> chunkserverList =
				getSecondaryListRequestMsg->getSecondaryList();
		waitAndDelete(getSecondaryListRequestMsg);
		return chunkserverList;
	}

	return {};
}

vector<bool> ChunkserverCommunicator::getChunkserverStatusRequest(vector<uint32_t> chunkserverIdList) {

	GetChunkserverStatusRequestMsg* getChunkserverStatusRequestMsg = new GetChunkserverStatusRequestMsg(
			this, getMonitorSockfd(), chunkserverIdList);
	getChunkserverStatusRequestMsg->prepareProtocolMsg();

	addMessage(getChunkserverStatusRequestMsg, true);
	MessageStatus status = getChunkserverStatusRequestMsg->waitForStatusChange();

	if (status == READY) {
		vector<bool> chunkserverStatusList = getChunkserverStatusRequestMsg->getChunkserverStatus();
		waitAndDelete(getChunkserverStatusRequestMsg);
		return chunkserverStatusList;
	}

	return {};
}

uint32_t ChunkserverCommunicator::sendBlockAck(uint64_t segmentId, uint32_t blockId,
		ComponentType dstComponent) {
	return 0;
}

//
// PRIVATE FUNCTIONS
//

void ChunkserverCommunicator::putBlockInit(uint32_t sockfd, uint64_t segmentId,
		uint32_t blockId, uint32_t length, uint32_t chunkCount,uint32_t blockIdOfDelta,uint32_t deltaId,
		DataMsgType dataMsgType, string updateKey) {

	// Step 1 of the upload process

	PutBlockInitRequestMsg* putBlockInitRequestMsg = new PutBlockInitRequestMsg(
			this, sockfd, segmentId, blockId, length, chunkCount, blockIdOfDelta,deltaId,dataMsgType,
			updateKey);

	putBlockInitRequestMsg->prepareProtocolMsg();
	addMessage(putBlockInitRequestMsg, true);

	MessageStatus status = putBlockInitRequestMsg->waitForStatusChange();
	if (status == READY) {
		waitAndDelete(putBlockInitRequestMsg);
		return;
	} else {
		debug_red("Put Block Init Failed %" PRIu64 ".%" PRIu32 "\n",
				segmentId, blockId);
		exit(-1);
	}

}

void ChunkserverCommunicator::putBlockData(uint32_t sockfd, uint64_t segmentId,
		uint32_t blockId, char* buf, uint64_t offset, uint32_t length,
		DataMsgType dataMsgType, string updateKey) {

	// Step 2 of the upload process
	
	BlockDataMsg* blockDataMsg = new BlockDataMsg(this, sockfd, segmentId,
			blockId, offset, length, dataMsgType, updateKey);

	blockDataMsg->prepareProtocolMsg();
	blockDataMsg->preparePayload(buf + offset, length);

	addMessage(blockDataMsg, false);
}

void ChunkserverCommunicator::putBlockEnd(uint32_t sockfd, uint64_t segmentId,
		uint32_t blockId, DataMsgType dataMsgType, string updateKey,
		vector<offset_length_t> offsetLength, vector<BlockLocation> parityList,
		CodingScheme codingScheme, string codingSetting, uint64_t segmentSize) {

	// Step 3 of the upload process

	BlockTransferEndRequestMsg* blockTransferEndRequestMsg =
			new BlockTransferEndRequestMsg(this, sockfd, segmentId, blockId,
					dataMsgType, updateKey, offsetLength, parityList,
					codingScheme, codingSetting, segmentSize);

	blockTransferEndRequestMsg->prepareProtocolMsg();
	addMessage(blockTransferEndRequestMsg, true);

	MessageStatus status = blockTransferEndRequestMsg->waitForStatusChange();
	if (status == READY) {
		waitAndDelete(blockTransferEndRequestMsg);
		return;
	} else {
		debug_error("Block Transfer End Failed %" PRIu64 ".%" PRIu32 "\n",
				segmentId, blockId);
		exit(-1);
	}
}

void ChunkserverCommunicator::segmentUploadAck(uint64_t segmentId, uint32_t segmentSize,
		CodingScheme codingScheme, string codingSetting,
		vector<uint32_t> nodeList) {
	uint32_t masterSockFd = getMasterSockfd();

	UploadSegmentAckMsg* uploadSegmentAckMsg = new UploadSegmentAckMsg(this,
			masterSockFd, segmentId, segmentSize, codingScheme, codingSetting,
			nodeList);

	uploadSegmentAckMsg->prepareProtocolMsg();
    uploadSegmentAckMsg->printProtocol();
	addMessage(uploadSegmentAckMsg, true);

    MessageStatus status = uploadSegmentAckMsg->waitForStatusChange();
    if(status == READY) {
        waitAndDelete(uploadSegmentAckMsg);
        return;
    }
    else {
        debug_error("Segment Upload Ack Failed [%" PRIu64 "]\n", segmentId);
        exit(-1);
    }
}

// DOWNLOAD

struct SegmentTransferChunkserverInfo ChunkserverCommunicator::getSegmentInfoRequest(
		uint64_t segmentId, uint32_t chunkserverId, bool needReply, bool isRecovery) {

	struct SegmentTransferChunkserverInfo segmentInfo = { };
	uint32_t masterSockFd = getMasterSockfd();

	GetSegmentInfoRequestMsg* getSegmentInfoRequestMsg =
			new GetSegmentInfoRequestMsg(this, masterSockFd, segmentId, chunkserverId,
					needReply, isRecovery);
	getSegmentInfoRequestMsg->prepareProtocolMsg();

	if (needReply) {
		addMessage(getSegmentInfoRequestMsg, true);

		MessageStatus status = getSegmentInfoRequestMsg->waitForStatusChange();
		if (status == READY) {
			segmentInfo._id = segmentId;
			segmentInfo._size = getSegmentInfoRequestMsg->getSegmentSize();
			segmentInfo._codingScheme =
					getSegmentInfoRequestMsg->getCodingScheme();
			segmentInfo._codingSetting =
					getSegmentInfoRequestMsg->getCodingSetting();
			segmentInfo._chunkserverList = getSegmentInfoRequestMsg->getNodeList();
			waitAndDelete(getSegmentInfoRequestMsg);
		} else {
			debug("Get Segment Info Request Failed %" PRIu64 "\n", segmentId);
			exit(-1);
		}

		return segmentInfo;
	} else {
		addMessage(getSegmentInfoRequestMsg);
		return {};
	}
}

void ChunkserverCommunicator::registerToMonitor(uint32_t ip, uint16_t port) {
	ChunkserverStartupMsg* startupMsg = new ChunkserverStartupMsg(this, getMonitorSockfd(),
			chunkserver->getChunkserverId(), chunkserver->getFreespace(), chunkserver->getCpuLoadavg(0), ip,
			port);
	startupMsg->prepareProtocolMsg();
	addMessage(startupMsg);
}

void ChunkserverCommunicator::repairBlockAck(uint64_t segmentId,
		vector<uint32_t> repairBlockList, vector<uint32_t> repairBlockChunkserverList) {

	RepairSegmentInfoMsg * repairSegmentInfoMsg = new RepairSegmentInfoMsg(this,
			getMasterSockfd(), segmentId, repairBlockList, repairBlockChunkserverList);
	repairSegmentInfoMsg->prepareProtocolMsg();
	addMessage(repairSegmentInfoMsg);
}
