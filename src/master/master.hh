/* 
 * File system MDPFS
 * master.hh
 * Copyright (c) 2022 Xiaosong Su 
 * All rights reserved.
 *
 */

#ifndef __MASTER_HH__
#define __MASTER_HH__

#include "metadatamodule.hh"
#include "namespacemodule.hh"
#include "master_communicator.hh"

#include <stdint.h>
#include <string.h>
#include <vector>

using namespace std;

class Master {
public:
	/**
	 * @brief	MASTER Constructor
	 */
	Master();

	~Master();

	/**
	 * @brief	Handle File Upload Request From Client
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client
	 * @param	dstPath		Target Path for the file uploaded
	 * @param	fileSize	Size of the File
	 * @param	numOfObjs	number of segments to be uploaded
	 *
	 * @return	File ID
	 */
	uint32_t uploadFileProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, const string &dstPath, uint64_t fileSize,
			uint32_t numOfObjs);

//	void FileSizeProcessor(uint32_t requestId, uint32_t connectionId, uint32_t fileId);

	/**
	 * @brief	Handle File Upload Request From Client
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client
	 * @param	fileId	File ID
	 * @param	path	File Path
	 */
	void deleteFileProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, uint32_t fileId, const string &path);

	/**
	 * @brief	Handle File Rename Request From Client
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client
	 * @param	fileId	File ID
	 * @param	path	File Path
	 * @param	newPath	New File Path
	 */
	void renameFileProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, uint32_t fileId, const string &path,
			const string &newPath);

	/**
	 * @brief	Handle Upload Segment Acknowledgement from Primary
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	segmentId	ID of the segment uploaded
	 * @param 	segmentSize	Size of segment
	 * @param	codingScheme	Coding Scheme
	 * @param 	codingSetting	Coding Scheme Setting
	 * @param	segmentNodeList	List of the Chunkserver
	 */
	void uploadSegmentAckProcessor(uint32_t requestId, uint32_t connectionId,
			uint64_t segmentId, uint32_t segmentSize, CodingScheme codingScheme,
			const string &codingSetting,
			const vector<uint32_t> &segmentNodeList);

	/**
	 * @brief	Handle Download File Request from Client (Request with Path)
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client Requesting
	 * @param	dstPath		Path of the file
	 */
	void downloadFileProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, const string &dstPath);

	/**
	 * @brief	Handle Download File Request from Client (Request with File ID)
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the Client
	 * @param	fileId		ID of the file
	 */
	void downloadFileProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, uint32_t fileId);

	/**
	 * @brief	Handle Get Segment ID Lsit
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the Client
	 * @param	numOfObjs	Number of Segments
	 */
	void getSegmentIdListProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, uint32_t numOfObjs);

	/**
	 * @brief	Handle Get File Info Request
	 *
	 * @param	requestId	Request ID
	 * @param	connectonId	Connection ID
	 * @param	clientId	ID of the Client
	 * @param	path	Path of the File
	 */
	void getFileInfoProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, const string &path);

	/**
	 * @brief	Handle the Segment Info Request from Chunkserver
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	segmentID	ID of the Segment
	 * @param 	chunkserverId	CHUNKSERVERID
	 */
	void getSegmentInfoProcessor(uint32_t requestId, uint32_t connectionId,
			uint64_t segmentId, uint32_t chunkserverId, bool needReply, bool isRecovery = false);

	/**
	 * @brief	Handle List Folder Request from Client
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the Client
	 * @param	path	Path to the Folder
	 */
	void listFolderProcessor(uint32_t requestId, uint32_t clientId,
			uint32_t connectionId, const string &path);

	/**
	 * @brief	Handle Secondary Node Failure Report from Chunkserver
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	chunkserverId		ID of the Failed Chunkserver
	 * @param	segmentId	ID of the Failed Segment
	 * @param	reason		Reason of the Failure (Default to Node Failure)
	 */
	void secondaryFailureProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t chunkserverId, uint64_t segmentId, FailureReason reason =
					UNREACHABLE);

	/**
	 * @brief	Handle Chunkserver Recovery Initialized by Monitor
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	chunkserverId		ID of the failed Chunkserver
	 */
	void recoveryTriggerProcessor(uint32_t requestId, uint32_t connectionId,
			vector<uint32_t> deadChunkserverList, bool dstSpecified, vector<uint32_t>
			dstChunkserverList);

	/**
	 * @brief	Handle Segment Node List Update from Chunkserver
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	segmentId	ID of the Segment
	 * @param	segmentNodeList	Node List of the Segment
	 */
	void nodeListUpdateProcessor(uint32_t requestId, uint32_t connectionId,
			uint64_t segmentId, const vector<uint32_t> &segmentNodeList);

	/**
	 * @brief	Handle Segment List Save Request
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client Requesting
	 * @param	fileId	ID of the File
	 * @param	segmentList	Segment List of the File
	 */
	void saveSegmentListProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, uint32_t fileId,
			const vector<uint64_t> &segmentList);

	/**
	 * @brief	Handle Set File Size Request
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client Requesting
	 * @param	fileId	ID of the File
	 * @param	fileSize	Size of the File
	 */
	void setFileSizeProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, uint32_t fileId, uint64_t fileSize);

	/**
	 * @brief Handle Ack from CHUNKSERVER after segment is recovered
	 * @param requestId Request ID
	 * @param connectionId Connection ID
	 * @param segmentId Segment ID
	 * @param repairBlockList List of repaired blocks
	 * @param repairBlockChunkserverList List of CHUNKSERVERs storing repaired blocks
	 */

	void repairSegmentInfoProcessor(uint32_t requestId, uint32_t connectionId,
			uint64_t segmentId, vector<uint32_t> repairBlockList,
			vector<uint32_t> repairBlockChunkserverList);

	/**
	 * @brief	Get the Master Communicator
	 *
	 * @return	Pointer to the Master Communicator Module
	 */
	MasterCommunicator* getCommunicator();

	/**
	 * @brief	Run the Master
	 */
	void run();

private:

	/**
	 * @brief	Process the Download Request
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client
	 * @param	fileId		ID of the File
	 * @param	path		Path of the File
	 */
	void downloadFileProcess(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, uint32_t fileId, const string &path);

	/// Handle Communication with other components
	MasterCommunicator* _masterCommunicator;

	/// Handle Metadata Operations
	MetaDataModule* _metaDataModule;

	/// Handle Namespace Operations
	NameSpaceModule* _nameSpaceModule;

	/// Running Indicator
	bool running;
	
	/// Segments Recovering
	//unordered_map<uint64_t, bool> _segmentRecoverStatus;
	uint32_t _numOfSegmentRepairRemaining;
	struct timeval _recoverStartTime;
    uint32_t _updateScheme;
};
#endif
