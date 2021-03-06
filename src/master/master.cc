/**
 * master.cc
 */

#include <cstdio>
#include <thread>
#include<iostream>
#include "master.hh"

#include "../common/garbagecollector.hh"
#include "../common/debug.hh"
#include "../config/config.hh"

using namespace std;

/**
 *	Global Variables
 */

/// Master Segment
Master* master;

/// Config Layer
ConfigLayer* configLayer;

/**
 * Initialise Master Communicator and MetaData Modules
 */
Master::Master() {
	_metaDataModule = new MetaDataModule();
	_nameSpaceModule = new NameSpaceModule();
	_masterCommunicator = new MasterCommunicator();
	configLayer = new ConfigLayer("chunkserverconfig.xml");
	_updateScheme = configLayer->getConfigInt("Storage>UpdateScheme");
}

Master::~Master() {
	delete _masterCommunicator;
	delete _metaDataModule;
	delete _nameSpaceModule;
}

/**
 * @brief	Handle File Upload Request From Client
 *
 * 1. Create File in the Name Space (Directory Tree) \n
 * 2. Create File Meta Data (Generate File ID) \n
 * 3. Generate Segment IDs \n
 * 4. Ask Monitor for Primary list \n
 * 5. Reply with Segment and Primary List
 */
uint32_t Master::uploadFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, const string &dstPath, uint64_t fileSize,
		uint32_t numOfObjs) {

	vector<uint64_t> segmentList(numOfObjs);
	vector<uint32_t> primaryList(numOfObjs);
	uint32_t fileId = 0;

	_nameSpaceModule->createFile(clientId, dstPath);
	fileId = _metaDataModule->createFile(clientId, dstPath, fileSize);

	segmentList = _metaDataModule->newSegmentList(numOfObjs);
	_metaDataModule->saveSegmentList(fileId, segmentList);

	//	primaryList = _masterCommunicator->askPrimaryList(numOfObjs);
	primaryList = _masterCommunicator->getPrimaryList(
			_masterCommunicator->getMonitorSockfd(), numOfObjs);
	for (uint32_t i = 0; i < primaryList.size(); i++) {
		debug("Get primary list index %" PRIu32 " = %" PRIu32 "\n",
				i, primaryList[i]);
	}

	_masterCommunicator->replySegmentandPrimaryList(requestId, connectionId,
			fileId, segmentList, primaryList);

	return fileId;
}

void Master::deleteFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId, const string &path) {

	string tmpPath = path;
	if (fileId != 0)
		tmpPath = _metaDataModule->lookupFilePath(fileId);
	else
		fileId = _metaDataModule->lookupFileId(tmpPath);

	debug("Delete File %s [%" PRIu32 "]\n", tmpPath.c_str(), fileId);
	_nameSpaceModule->deleteFile(clientId, tmpPath);
	_metaDataModule->deleteFile(clientId, fileId);
	_masterCommunicator->replyDeleteFile(requestId, connectionId, fileId);
}

/**
 * @brief	Handle File Rename Request From Client
 */
void Master::renameFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId, const string &path,
		const string &newPath) {

	string tmpPath = path;
	if (fileId != 0)
		tmpPath = _metaDataModule->lookupFilePath(fileId);
	else
		fileId = _metaDataModule->lookupFileId(tmpPath);

	_nameSpaceModule->renameFile(clientId, tmpPath, newPath);
	_metaDataModule->renameFile(clientId, fileId, newPath);
	_masterCommunicator->replyRenameFile(requestId, connectionId, fileId, tmpPath);
}

/**
 * @brief	Handle Upload Segment Acknowledgement from Primary
 *
 * 1. Save the Node List of the segment \n
 * 2. Set the Primary for the segment
 */
/*
   void Master::FileSizeProcessor(uint32_t requestId, uint32_t connectionId, uint32_t fileId){
   uint64_t fileSize = _metaDataModule->readFileSize(fileId);
   _masterCommunicator->replyFileSize(requestId, connectionId, fileId, fileSize);
   }
 */

void Master::uploadSegmentAckProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t segmentId, uint32_t segmentSize, CodingScheme codingScheme,
		const string &codingSetting, const vector<uint32_t> &segmentNodeList) {
	struct SegmentMetaData segmentMetaData;
	segmentMetaData._id = segmentId;
	segmentMetaData._nodeList = segmentNodeList;
	segmentMetaData._primary = segmentNodeList[0];
	segmentMetaData._codingScheme = codingScheme;
	segmentMetaData._codingSetting = codingSetting;
	segmentMetaData._size = segmentSize;
    _metaDataModule->saveSegmentInfoToCache(segmentId, segmentMetaData);
    _masterCommunicator->replyUploadSegmentAck(requestId, connectionId, segmentId);
	_metaDataModule->saveSegmentInfo(segmentId, segmentMetaData);
	//_metaDataModule->saveNodeList(segmentId, segmentNodeList);
	//_metaDataModule->setPrimary(segmentId, segmentNodeList[0]);

	return;
}

/**
 * @brief	Handle Download File Request from Client (Request with Path)
 */
void Master::downloadFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, const string &dstPath) {
	uint32_t fileId = _metaDataModule->lookupFileId(dstPath);
	debug("Path = %s [%" PRIu32 "]\n", dstPath.c_str(), fileId);
	return downloadFileProcess(requestId, connectionId, clientId, fileId,
			dstPath);
}

/**
 * @brief	Handle Download File Request from Client (Request with File ID)
 */
void Master::downloadFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId) {
	string path = _metaDataModule->lookupFilePath(fileId);
	return downloadFileProcess(requestId, connectionId, clientId, fileId, path);
}

/**
 * @brief	Process the Download Request
 *
 * 1. Open File in the Name Space \n
 * 2. Open File Metadata \n
 * 3. Read Segment List from the Metadata Module \n
 * 4. Read Primary Node ID for each Segment \n
 * 5. Reply with Segment and Primary List
 */
void Master::downloadFileProcess(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId, const string &path) {
	vector<uint64_t> segmentList;
	vector<uint32_t> primaryList;
	uint64_t fileSize = 0;
	FileType fileType = NORMAL;

	_nameSpaceModule->openFile(clientId, path);
	_metaDataModule->openFile(clientId, fileId);
	if (fileId != 0) {
		debug("Read segment List %" PRIu32 "\n", fileId);
		segmentList = _metaDataModule->readSegmentList(fileId);

		vector<uint64_t>::iterator it;
		uint32_t primaryId;
		for (it = segmentList.begin(); it < segmentList.end(); ++it) {

            debug("Read primary list %" PRIu64 "\n", *it);
            try {
                primaryId = _metaDataModule->getPrimary(*it);
                if (_masterCommunicator->getSockfdFromId(primaryId) == (uint32_t) -1) {
                    // if currently fail
                    vector<uint32_t> nodeList = _metaDataModule->readNodeList(*it);
                    while (true) {
                        uint32_t idx = rand() % nodeList.size();
                        if (_masterCommunicator->getSockfdFromId(nodeList[idx]) != (uint32_t) -1) {
                            primaryId = nodeList[idx];
                            break;
                        }
                    }

                }

            } catch (...) {
                debug_yellow("%s\n", "No Primary Found");
                continue;
            }
            primaryList.push_back(primaryId);
		}
		segmentList.resize(primaryList.size());

		fileSize = _metaDataModule->readFileSize(fileId);

		debug("FILESIZE = %" PRIu64 "\n", fileSize);
	} else {
		fileType = NOTFOUND;
	}

	_masterCommunicator->replyDownloadInfo(requestId, connectionId, fileId, path,
			fileSize, fileType, segmentList, primaryList);

	return;
}

/**
 * @brief	Handle Get Segment ID Lsit
 */
void Master::getSegmentIdListProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t numOfObjs) {
	vector<uint64_t> segmentList = _metaDataModule->newSegmentList(numOfObjs);
	vector<uint32_t> primaryList = _masterCommunicator->getPrimaryList(
			_masterCommunicator->getMonitorSockfd(), numOfObjs);
	_masterCommunicator->replySegmentIdList(requestId, connectionId, segmentList,
			primaryList);
	return;
}

/**
 * @brief	Handle Get File Info Request
 */
void Master::getFileInfoProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, const string &path) {
	downloadFileProcessor(requestId, connectionId, clientId, path);

	return;
}

/**
 * @brief	Handle the Segment Info Request from Chunkserver
 * TODO: Currently Only Supplying Info Same as Download
 */
void Master::getSegmentInfoProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t segmentId, uint32_t chunkserverId, bool needReply, bool isRecovery) {

	struct SegmentMetaData segmentMetaData = _metaDataModule->readSegmentInfo(
			segmentId);

	if (needReply) {
		_masterCommunicator->replySegmentInfo(requestId, connectionId, segmentId,
				segmentMetaData._size, segmentMetaData._nodeList,
				segmentMetaData._codingScheme, segmentMetaData._codingSetting);
	}

	return;
}


/**
 * @brief	Handle List Folder Request from Client
 *
 * 1. List Folder with Name Space Module \n
 * 2. Reply with Folder Data
 */
void Master::listFolderProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, const string &path) {
	vector<FileMetaData> folderData;

	debug("List %s by %" PRIu32 "\n", path.c_str(), clientId);
	folderData = _nameSpaceModule->listFolder(clientId, path);
	_masterCommunicator->replyFolderData(requestId, connectionId, path,
			folderData);

	return;
}

/**
 * @brief	Handle Secondary Node Failure Report from Chunkserver
 */
void Master::secondaryFailureProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t chunkserverId, uint64_t segmentId, FailureReason reason) {
	_masterCommunicator->reportFailure(chunkserverId, reason);

	return;
}

/**
 * @brief	Handle Chunkserver Recovery Initialized by Monitor
 *
 * 1. Read Segment List of the Failed Chunkserver
 * 2. For each Segment, Read Current Primary and Node List
 * 3. Reply with Segment List, Primary List, and Node List of the Segments
 */
void Master::recoveryTriggerProcessor(uint32_t requestId, uint32_t connectionId,
		vector<uint32_t> deadChunkserverList, bool dstSpecified, vector<uint32_t>
		dstChunkserverList) {

	set <uint64_t> recoverySegments;

	debug_yellow("%s\n", "Recovery Triggered ");
	gettimeofday(&_recoverStartTime, NULL);

	// this map is used iff dstSpecified = true
	map<uint32_t, uint32_t> mapped;

	if (dstSpecified) {
		// one to one map from dead to dst
		sort(deadChunkserverList.begin(), deadChunkserverList.end());
		sort(dstChunkserverList.begin(), dstChunkserverList.end());
		for (int i = 0; i < (int)deadChunkserverList.size(); i++) {
			mapped[deadChunkserverList[i]] = dstChunkserverList[i%dstChunkserverList.size()];
		}
	}

	vector<struct SegmentLocation> segmentLocationList;

	struct SegmentLocation segmentLocation;

	for (uint32_t chunkserverId : deadChunkserverList) {

		// get the list of segments owned by the failed chunkserver as primary
		vector<uint64_t> primarySegmentList =
			_metaDataModule->readChunkserverPrimarySegmentList(chunkserverId);

		for (auto segmentId : primarySegmentList) {
			cout<<chunkserverId<<endl<<segmentId<<endl;
			if (dstSpecified) {

				// pre-assign the newly start chunkserver as primary
				_metaDataModule->setPrimary(segmentId, mapped[chunkserverId]);

			} else {

				// get the node list of an segment and their status
				vector<uint32_t> nodeList = _metaDataModule->readNodeList(
						segmentId);
				vector<bool> nodeStatus = _masterCommunicator->getChunkserverStatusRequest(
						nodeList);

				// select new primary CHUNKSERVER and write to DB
				_metaDataModule->selectActingPrimary(segmentId, nodeList,
						nodeStatus);
			}
		}
	}

	for (uint32_t chunkserverId : deadChunkserverList) {

		// get the list of segments owned by failed chunkserver
		vector<uint64_t> segmentList = _metaDataModule->readChunkserverSegmentList(
				chunkserverId);

		for (auto segmentId : segmentList) {
			if (!recoverySegments.count(segmentId)) {
				debug_cyan("Check segmentid = %" PRIu64 "\n", segmentId);
				segmentLocation.segmentId = segmentId;
				segmentLocation.chunkserverList = _metaDataModule->readNodeList(
						segmentId);
				segmentLocation.primaryId = _metaDataModule->getPrimary(
						segmentId);
				segmentLocationList.push_back(segmentLocation);
				recoverySegments.insert(segmentId);
			}
		}
	}

	_numOfSegmentRepairRemaining = segmentLocationList.size();

	_masterCommunicator->replyRecoveryTrigger(requestId, connectionId,
			segmentLocationList);

	return;
}

/**
 * @brief	Handle Segment Node List Update from Chunkserver
 */
void Master::nodeListUpdateProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t segmentId, const vector<uint32_t> &segmentNodeList) {
	_metaDataModule->saveNodeList(segmentId, segmentNodeList);
	_metaDataModule->setPrimary(segmentId, segmentNodeList[0]);
	return;
}

/**
 * @brief	Handle Segment List Save Request
 */
void Master::saveSegmentListProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId,
		const vector<uint64_t> &segmentList) {
	_metaDataModule->saveSegmentList(fileId, segmentList);
	_masterCommunicator->replySaveSegmentList(requestId, connectionId, fileId);
	return;
}

void Master::repairSegmentInfoProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t segmentId, vector<uint32_t> repairBlockList,
		vector<uint32_t> repairBlockChunkserverList) {
	struct SegmentMetaData segmentMetaData = _metaDataModule->readSegmentInfo(
			segmentId);

	for (int i = 0; i < (int) repairBlockList.size(); i++) {
		segmentMetaData._nodeList[repairBlockList[i]] = repairBlockChunkserverList[i];
	}

	_metaDataModule->saveNodeList(segmentId, segmentMetaData._nodeList);

	_numOfSegmentRepairRemaining--;
	if(_numOfSegmentRepairRemaining == 0) {
		struct timeval _recoverEndTime;
		gettimeofday(&_recoverEndTime, NULL);
		long milliseconds = (_recoverEndTime.tv_usec - _recoverStartTime.tv_usec) / 1000;
		if (milliseconds < 0) {
			milliseconds += 1000;
			_recoverEndTime.tv_sec--;
		}
		long seconds = _recoverEndTime.tv_sec - _recoverStartTime.tv_sec;
        double second = seconds+milliseconds*0.001;
        double percentage = 0.15/(1-0.15);
        __useconds_t SLEEP = (int)(second*1000000*percentage);
        if (_updateScheme != MPLR){
            usleep(SLEEP);
        }
        gettimeofday(&_recoverEndTime, NULL);
        long millisecondsx = (_recoverEndTime.tv_usec - _recoverStartTime.tv_usec) / 1000;
        if (millisecondsx < 0) {
            millisecondsx += 1000;
            _recoverEndTime.tv_sec--;
        }
        long secondsx = _recoverEndTime.tv_sec - _recoverStartTime.tv_sec;
        printf ("[RECOVER] Recovery Ends. Duration = %ld.%ld seconds\n", secondsx, millisecondsx);
	}
}

MasterCommunicator* Master::getCommunicator() {
	return _masterCommunicator;
}

/**
 * @brief	Handle Set File Size Request
 */
void Master::setFileSizeProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId, uint64_t fileSize) {
	_metaDataModule->setFileSize(fileId, fileSize);

	return;
}


int main(void) {
	configLayer = new ConfigLayer("masterconfig.xml");
	master = new Master();

	MasterCommunicator* communicator = master->getCommunicator();

	communicator->createServerSocket();
	communicator->setId(50000);
	communicator->setComponentType(MASTER);

	// 1. Garbage Collection Thread (lamba function hack for singleton)
	thread garbageCollectionThread(
			[&]() {GarbageCollector::getInstance().start();});

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, communicator);


	communicator->connectToMonitor();

	garbageCollectionThread.join();
	receiveThread.join();

	delete master;
	delete configLayer;
	return 0;
}
