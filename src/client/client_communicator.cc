#include <iostream>
#include <thread>
#include "client_communicator.hh"
#include "client.hh"

#include "../common/debug.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"
#include "../protocol/metadata/listdirectoryrequest.hh"
#include "../protocol/metadata/uploadfilerequest.hh"
#include "../protocol/metadata/deletefilerequest.hh"
#include "../protocol/metadata/downloadfilerequest.hh"
#include "../protocol/metadata/savesegmentlistrequest.hh"
#include "../protocol/metadata/setfilesizerequest.hh"
#include "../protocol/metadata/renamefilerequest.hh"
#include "../protocol/transfer/putsegmentinitrequest.hh"
#include "../protocol/transfer/putsegmentinitreply.hh"
#include "../protocol/transfer/segmenttransferendrequest.hh"
#include "../protocol/transfer/segmenttransferendreply.hh"
#include "../protocol/transfer/segmentdatamsg.hh"
#include "../protocol/metadata/getsegmentidlistrequest.hh"
#include "../protocol/transfer/getsegmentrequest.hh"
#include "../protocol/nodelist/getchunkserverlistrequest.hh"
#include "../protocol/nodelist/getchunkserverlistreply.hh"

/**
 * @brief	Send List Folder Request to Master (Blocking)
 *
 * 1. Create List Directory Request Message \n
 * 2. Use Future to Wait until Reply is Back
 */
extern Client* client;

struct FileMetaData ClientCommunicator::uploadFile(uint32_t clientId,
		string path, uint64_t fileSize, uint32_t numOfObjs) {

	uint32_t masterSockFd = getMasterSockfd();
	UploadFileRequestMsg* uploadFileRequestMsg = new UploadFileRequestMsg(this,
			masterSockFd, clientId, path, fileSize, numOfObjs);
	uploadFileRequestMsg->prepareProtocolMsg();

	addMessage(uploadFileRequestMsg, true);
	MessageStatus status = uploadFileRequestMsg->waitForStatusChange();

	if (status == READY) {
		struct FileMetaData fileMetaData { };
		fileMetaData._id = uploadFileRequestMsg->getFileId();
        fileMetaData._path = path;
		fileMetaData._segmentList = uploadFileRequestMsg->getSegmentList();
		fileMetaData._primaryList = uploadFileRequestMsg->getPrimaryList();
		waitAndDelete(uploadFileRequestMsg);
		return fileMetaData;
	} else {
		debug("%s\n", "Upload File Request Failed");
		exit(-1);
	}
	return {};
}

void ClientCommunicator::deleteFile(uint32_t clientId, string path, uint32_t fileId) {
	uint32_t masterSockFd = getMasterSockfd();
	DeleteFileRequestMsg* deleteFileRequestMsg = new DeleteFileRequestMsg(this, masterSockFd, clientId, fileId, path);
	deleteFileRequestMsg->prepareProtocolMsg();
	addMessage(deleteFileRequestMsg, true);
	MessageStatus status = deleteFileRequestMsg->waitForStatusChange();

	if (status == READY){
		return ;
	} else {
		debug_yellow("Delete File Request Failed %s [%" PRIu32 "]\n", path.c_str(), fileId);
		exit(-1);
	}
	return ;
}

struct FileMetaData ClientCommunicator::downloadFile(uint32_t clientId,
		uint32_t fileId) {
	uint32_t masterSockFd = getMasterSockfd();
	DownloadFileRequestMsg* downloadFileRequestMsg = new DownloadFileRequestMsg(
			this, masterSockFd, clientId, fileId);
	downloadFileRequestMsg->prepareProtocolMsg();

	addMessage(downloadFileRequestMsg, true);
	MessageStatus status = downloadFileRequestMsg->waitForStatusChange();

	if (status == READY) {
		struct FileMetaData fileMetaData { };
		fileMetaData._id = downloadFileRequestMsg->getFileId();
		fileMetaData._path = downloadFileRequestMsg->getFilePath();
		fileMetaData._size = downloadFileRequestMsg->getSize();
		fileMetaData._segmentList = downloadFileRequestMsg->getSegmentList();
		fileMetaData._primaryList = downloadFileRequestMsg->getPrimaryList();
		fileMetaData._fileType = downloadFileRequestMsg->getFileType();
		waitAndDelete(downloadFileRequestMsg);
		return fileMetaData;
	} else {
		debug("%s\n", "Download File Request Failed");
		exit(-1);
	}
	return {};
}

struct FileMetaData ClientCommunicator::downloadFile(uint32_t clientId,
		string filePath) {
	uint32_t masterSockFd = getMasterSockfd();
	DownloadFileRequestMsg* downloadFileRequestMsg = new DownloadFileRequestMsg(
			this, masterSockFd, clientId, filePath);
	downloadFileRequestMsg->prepareProtocolMsg();

	addMessage(downloadFileRequestMsg, true);
	MessageStatus status = downloadFileRequestMsg->waitForStatusChange();

	if (status == READY) {
		struct FileMetaData fileMetaData { };
		fileMetaData._id = downloadFileRequestMsg->getFileId();
		fileMetaData._path = downloadFileRequestMsg->getFilePath();
		fileMetaData._size = downloadFileRequestMsg->getSize();
		fileMetaData._segmentList = downloadFileRequestMsg->getSegmentList();
		fileMetaData._primaryList = downloadFileRequestMsg->getPrimaryList();
		waitAndDelete(downloadFileRequestMsg);
		return fileMetaData;
	} else {
		debug("%s\n", "Download File Request Failed");
		exit(-1);
	}
	return {};
}

void ClientCommunicator::renameFile(uint32_t clientId, uint32_t fileId, const string& path, const string& newPath) {
	uint32_t masterSockFd = getMasterSockfd();
	RenameFileRequestMsg* renameFileRequestMsg = new RenameFileRequestMsg(this, masterSockFd, clientId, fileId, path, newPath);
	renameFileRequestMsg->prepareProtocolMsg();

	addMessage(renameFileRequestMsg, true);
	MessageStatus status = renameFileRequestMsg->waitForStatusChange();

	if(status == READY) {
		return ;
	} else {
		debug_error("Rename File Failed %s[%" PRIu32 "] %s\n",path.c_str(),fileId,newPath.c_str());
		exit (-1);
	}
	return ;
}

/**
 * @brief	Get File Info
 * TODO: Currently Doing Same as Download
 */
struct FileMetaData ClientCommunicator::getFileInfo(uint32_t clientId,
		uint32_t fileId) {
	return downloadFile(clientId, fileId);
}

/**
 *
 * @brief	Get File Info
 * TODO: Currently Doing Same as Download
 */
struct FileMetaData ClientCommunicator::getFileInfo(uint32_t clientId,
		string filePath) {
	return downloadFile(clientId, filePath);
}

void ClientCommunicator::saveSegmentList(uint32_t clientId, uint32_t fileId,
		vector<uint64_t> segmentList) {
	uint32_t masterSockfd = getMasterSockfd();
	SaveSegmentListRequestMsg* saveSegmentListRequestMsg =
			new SaveSegmentListRequestMsg(this, masterSockfd, clientId, fileId,
					segmentList);
	saveSegmentListRequestMsg->prepareProtocolMsg();

	addMessage(saveSegmentListRequestMsg, true);

	MessageStatus status = saveSegmentListRequestMsg->waitForStatusChange();

	if (status == READY) {
		return ;
	} else {
		debug("Save Segment List Request Failed [%" PRIu32 "]\n",fileId);
		exit(-1);
	}
	return;
}

vector<struct SegmentMetaData> ClientCommunicator::getNewSegmentList (uint32_t clientId, uint32_t numOfObjs)
{
	uint32_t masterSockfd = getMasterSockfd();
	debug("Requesting New Segment Id, Number of Segments %" PRIu32 "\n", numOfObjs);
	GetSegmentIdListRequestMsg* getSegmentIdListRequestMsg = new GetSegmentIdListRequestMsg(this, masterSockfd, clientId, numOfObjs);
	getSegmentIdListRequestMsg->prepareProtocolMsg();

	addMessage(getSegmentIdListRequestMsg,true);

	MessageStatus status = getSegmentIdListRequestMsg->waitForStatusChange();

	if(status == READY) {
		vector<struct SegmentMetaData> segmentMetaDataList;
		vector<uint32_t> primaryList = getSegmentIdListRequestMsg->getPrimaryList();
		vector<uint64_t> segmentList = getSegmentIdListRequestMsg->getSegmentIdList();
		for(uint32_t i = 0; i < primaryList.size(); ++i){
			struct SegmentMetaData tempSegmentMetaData;
			tempSegmentMetaData._id =  segmentList[i];
			tempSegmentMetaData._primary = primaryList[i];
			segmentMetaDataList.push_back(tempSegmentMetaData);
		}
		return segmentMetaDataList;
	} else {
		debug("%s\n","Get New Segment List Failed");
		exit(-1);
	}

	return {};
}

void ClientCommunicator::saveFileSize(uint32_t clientId, uint32_t fileId, uint64_t fileSize)
{
	uint32_t masterSockfd = getMasterSockfd();
	SetFileSizeRequestMsg* setFileSizeRequestMsg = new SetFileSizeRequestMsg(this, masterSockfd, clientId, fileId, fileSize);
	
	setFileSizeRequestMsg->prepareProtocolMsg();
	addMessage(setFileSizeRequestMsg);
	return ;
}

void ClientCommunicator::replyPutSegmentInit(uint32_t requestId,
		uint32_t connectionId, uint64_t segmentId) {

	PutSegmentInitReplyMsg* putSegmentInitReplyMsg = new PutSegmentInitReplyMsg(
			this, requestId, connectionId, segmentId);
	putSegmentInitReplyMsg->prepareProtocolMsg();

	addMessage(putSegmentInitReplyMsg);
}

void ClientCommunicator::replyPutSegmentEnd(uint32_t requestId,
		uint32_t connectionId, uint64_t segmentId, bool isSmallSegment) {
	SegmentTransferEndReplyMsg* putSegmentEndReplyMsg =
			new SegmentTransferEndReplyMsg(this, requestId, connectionId,
					segmentId, isSmallSegment);
	putSegmentEndReplyMsg->prepareProtocolMsg();

	debug("Reply put segment end for ID: %" PRIu64 "\n", segmentId);
	addMessage(putSegmentEndReplyMsg);
}

void ClientCommunicator::requestSegment(uint32_t dstSockfd, uint64_t segmentId) {
	GetSegmentRequestMsg* getSegmentRequestMsg = new GetSegmentRequestMsg(this,
			dstSockfd, segmentId);

	getSegmentRequestMsg->prepareProtocolMsg();
	addMessage(getSegmentRequestMsg);
}

void ClientCommunicator::getChunkserverListAndConnect() {
	GetChunkserverListRequestMsg* getChunkserverListRequestMsg = new GetChunkserverListRequestMsg(this,
		getMonitorSockfd());
	getChunkserverListRequestMsg->prepareProtocolMsg();
	addMessage(getChunkserverListRequestMsg, true);
	MessageStatus status = getChunkserverListRequestMsg->waitForStatusChange();

	if (status == READY) {
		vector<struct OnlineChunkserver>& onlineList =
				getChunkserverListRequestMsg->getChunkserverList();
		for (uint32_t i = 0; i < onlineList.size(); i++)
			if (_forwardMode) {
				sockaddr_in m_addr;
				inet_pton(AF_INET, _forwardIp.c_str(), &m_addr.sin_addr);
				uint32_t ip = m_addr.sin_addr.s_addr;
				connectToChunkserver(ip, onlineList[i].chunkserverPort);
			} else
				connectToChunkserver(onlineList[i].chunkserverIp, onlineList[i].chunkserverPort);
	}
	
}
