/* 
 * File system MDPFS
 * chunkserver.hh
 * Copyright (c) 2022 Xiaosong Su 
 * All rights reserved.
 *
 */

#ifndef __CHUNKSERVER_HH__
#define __CHUNKSERVER_HH__
#include <stdint.h>
#include <vector>
#include <set>
#include "chunkserver_communicator.hh"
#include "storagemodule.hh"
#include "codingmodule.hh"
#include "../common/metadata.hh"
#include "../common/segmentdata.hh"
#include "../common/blockdata.hh"
#include "../common/blocklocation.hh"
#include "../common/onlinechunkserver.hh"
#include "../protocol/message.hh"
#include "../datastructure/concurrentmap.hh"

/**
 * Central class of CHUNKSERVER
 * All functions of CHUNKSERVER are invoked here
 * Segments and Blocks can be divided into trunks for transportation
 */

/**
 * Message Functions
 *
 * UPLOAD
 * 1. putSegmentProcessor
 * 2. segmentTrunkProcessor 	-> getChunkserverListRequest (MONITOR)
 * 									-> chunkserverListProcessor
 * 							-> sendBlockToChunkserver
 * 3. (other CHUNKSERVER) putBlockProcessor
 * 4. (other CHUNKSERVER) blockTrunkProcessor
 * 5. sendBlockAck (PRIMARY CHUNKSERVER, CLIENT, Master)
 *
 * DOWNLOAD
 * 1. getSegmentProcessor 	-> getChunkserverListRequest (Master)
 * 									-> chunkserverListProcessor
 * 2. getBlockRequest
 * 			-> (other CHUNKSERVER) getBlockProcessor
 * 			-> (other CHUNKSERVER) sendBlockToChunkserver
 * 			-> putBlockProcessor
 * 			-> blockTrunkProcessor
 * 	3. sendSegmentToClient
 */

class Chunkserver {
public:

    /**
     * Constructor
     */

    Chunkserver(uint32_t selfId);

    /**
     * Destructor
     */

    ~Chunkserver();

    /**
     * Action when an CHUNKSERVER list is received
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param segmentId 	Segment ID
     * @param chunkserverList 	Secondary CHUNKSERVER List
     * @return Length of list if success, -1 if failure
     */

    uint32_t chunkserverListProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, vector<struct BlockLocation> chunkserverList);

    // DOWNLOAD

    /**
     * Action when a getSegmentRequest is received
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param segmentId 	ID of the segment to send
     * @param localRetrieve (Optional) Retrieve the segment and store in cache
     */
	 
    void getSegmentRequestProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, bool localRetrieve = false);

    /**
     * Action when a getBlockRequest is received
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param symbols List of symbols to retrieve
     * @param dataMsgType Data Msg Type
     */

    void getBlockRequestProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, uint32_t blockId,
            vector<offset_length_t> symbols, DataMsgType dataMsgType, bool isParity);

    /**
    * Action when a getDeltaRequest is received
    * @param requestId Request ID
    * @param sockfd Socket descriptor of message source
    * @param segmentId Segment ID
    * @param blockId Block ID
    * @param getDeltaNum begin delta id
    * @param allDeltaNum all need delta id
    * @param dataMsgType Data Msg Type
    */

    void getDeltaRequestProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, uint32_t blockId,uint32_t getDeltaNum,
            uint32_t allDeltaNum,DataMsgType dataMsgType);

    /**
     * Action when a getRecoveryBlockRequest is received
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param symbols List of symbols to retrieve
     */

    void getRecoveryBlockProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, uint32_t blockId,
            vector<offset_length_t> symbols);

    /**
     * Get a recovery block from another CHUNKSERVER
     * @param recoverytpId Map Key for recoverytpRequestCount
     * @param chunkserverId ID of the CHUNKSERVER to retrieve from
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param offsetLength List of <offset, length> in the target block
     * @param repairedBlock Referenced memory which the recovered block is stored
     */
    void retrieveRecoveryBlock(uint32_t recoverytpId, uint32_t chunkserverId,
            uint64_t segmentId, uint32_t blockId,
            vector<offset_length_t> &offsetLength, BlockData &repairedBlock, bool isParity);

    /**
     * Get a recovery Delta from another CHUNKSERVER
     * @param recoverytpId Map Key for recoverytpRequestCount
     * @param chunkserverId ID of the CHUNKSERVER to retrieve from
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param offsetLength List of <offset, length> in the target block
     * @param repairedBlock Referenced memory which the recovered block is stored
     */
    void retrieveRecoveryDelta(uint32_t recoverytpId, uint32_t chunkserverId,uint64_t segmentId, uint32_t blockId,
            uint32_t beginDeltaId,uint32_t endDeltaId,vector<BlockData> &delta);

    /**
     * Action when a put segment request is received
     * A number of trunks are expected to receive afterwards
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param segmentId Segment ID
     * @param length Segment size, equals the total length of all the trunks
     * @param chunkCount number of chunks that will be received
     * @param codingScheme Coding Scheme for the segment
     * @param setting Coding setting for the segment
     * @param updateKey Key for UPDATE message
     */

    DataMsgType putSegmentInitProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, uint32_t segLength, uint32_t bufLength,
            uint32_t chunkCount, CodingScheme codingScheme, string setting,
            string updateKey, bool isSmallSegment = false);

    /**
     * Action when a put segment end is received
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param segmentId Segment ID
     * @param dataMsgType Data Msg Type
     * @param updateKey Update Key
     * @param offsetlength <Offset, Length> of chunks in the segment
     */

    void putSegmentEndProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, DataMsgType dataMsgType, string updateKey,
            vector<offset_length_t> offsetLength,uint32_t ifrom, bool isSmallSegment = false);

    /**
     * Action when an segment trunk is received
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param segmentId Segment ID
     * @param offset Offset of the trunk in the segment
     * @param length Length of trunk
     * @param dataMsgType Data Msg Type
     * @param updateKey Update Key
     * @param buf Pointer to buffer
     * @return Length of trunk if success, -1 if failure
     */

    uint32_t putSegmentDataProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, uint64_t offset, uint32_t length,
            DataMsgType dataMsgType, string updateKey, char* buf);

    /**
     * Action when a putBlockInitRequest is received
     * A number of trunks are expected to be received afterwards
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param length Block size, equals the total length of all the trunks
     * @param chunkCount No of trunks to receive
     */

    void putBlockInitProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, uint32_t blockId, uint32_t length,
            uint32_t chunkCount, uint32_t blockIdOfDelta,uint32_t deltaId, DataMsgType dataMsgType, string updateKey);

    /**
     * Distribute Blocks to CHUNKSERVER
     *
     * @param segmentId	Segment Id
     * @param blockData	Data Block
     * @param blockLocation Location of Block
     * @param blocktpId Map key of blocktpRequestCount
     */
	 
    void distributeBlock(uint64_t segmentId, const struct BlockData blockData,
            const struct BlockLocation& blockLocation, DataMsgType dataMsgType,
            uint32_t blocktpId = 0);

    /**
     * Action when a block trunk is received
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param offset Offset of the trunk in the block
     * @param length Length of trunk
     * @param buf Pointer to buffer
     * @return Length of trunk if success, -1 if failure
     */

    uint32_t putBlockDataProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, uint32_t blockId, uint32_t offset,
            uint32_t length, char* buf, DataMsgType dataMsgType,
            string updateKey);

    vector<BlockData> computeDelta(uint64_t segmentId, uint32_t blockId,
        BlockData newBlock, vector<offset_length_t> offsetLength, vector<uint32_t> parityVector);
    void sendDelta(uint64_t segmentId, uint32_t blockId, BlockData newBlock,
            vector<offset_length_t> offsetLength);

    /**
     * Action when a put block end is received
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param segmentId Segment ID
     * @param blockId Block ID
     */

    void putBlockEndProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, uint32_t blockId, DataMsgType dataMsgType,
            string updateKey, vector<offset_length_t> offsetLength,
            vector<BlockLocation> parityList, CodingScheme codingScheme,
            string codingSetting, uint64_t segmentSize);

    /**
     * Action when a recovery request is received
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param segmentId Segment ID
     * @param repairBlockId Block ID to be repaired
     * @param repairBlockChunkserver New CHUNKSERVERs to store the repaired blocks
     */

    void repairSegmentInfoProcessor(uint32_t requestId, uint32_t sockfd,
            uint64_t segmentId, vector<uint32_t> repairBlockId,
            vector<uint32_t> repairBlockChunkserver);

    /**
     * Action when a monitor requests a status update
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     */
    void ChunkserverStatUpdateRequestProcessor(uint32_t requestId, uint32_t sockfd);

    /**
     * Action when a monitor tells a new chunkserver is startup,to connect it if my id > its id
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param chunkserverId the id of the newly startup chunkserver
     * @param chunkserverIp the ip addr of the newly startup chunkserver
     * @param chunkserverPort the port of the newly startup chunkserver
     */
    void NewChunkserverRegisterProcessor(uint32_t requestId, uint32_t sockfd,
            uint32_t chunkserverId, uint32_t chunkserverIp, uint32_t chunkserverPort);

    /**
     * Action when a monitor flush the online list to a chunkserver
     * @param requestId Request ID
     * @param sockfd Socket descriptor of message source
     * @param onlineChunkserverList list reference contains all the online chunkserver with its<ip, port, id>
     */
    void OnlineChunkserverListProcessor(uint32_t requestId, uint32_t sockfd,
            vector<struct OnlineChunkserver>& onlineChunkserverList);


    // getters

    /**
     * To get the current cpu load average in last 15 mins. If error, return
     * infinity INF = (1<<29)
     * @param idx 0 to get last one minute, 1 to get last 5 mins, 2 to get last 15 mins
     * @return loading*100 to cast into integer
     */
    uint32_t getCpuLoadavg(int idx);
    bool ifRepairParity(uint32_t blockDataNum,vector<uint32_t> repairBlockList );
    /**
     * To get the free space of the current disk in MB
     * @return free space in MB, if error, return 0
     */
    uint64_t getFreespace();

    /**
     * Get a reference of CHUNKSERVERCommunicator
     * @return Pointer to CHUNKSERVER communication module
     */

    ChunkserverCommunicator* getCommunicator();

    /**
     * Get a reference of StorageModule
     * @return Pointer to CHUNKSERVER storage module
     */

    StorageModule* getStorageModule();

    /**
     * Get the ID
     * @return CHUNKSERVER ID
     */

    uint32_t getChunkserverId();

    /**
     * If block is not requested, return false and set status to true
     * If block is requested, return true
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @return is block requested
     */

    bool isBlockRequested(uint64_t segmentId, uint32_t blockId);

    void dumpLatency();

private:

    /**
     * Set the bool array containing the status of the CHUNKSERVERs holding the blocks
     * @param chunkserverListStatus Bool array representing CHUNKSERVER status (true = up, false = down)
     */

    //	void setChunkserverListStatus (vector<bool> &secondaryChunkserverStatus);
    /**
     * Retrieve a block from the storage
     * @param segmentId ID of the segment that the block is belonged to
     * @param blockId Target Block ID
     * @return BlockData structure
     */

    struct BlockData getBlockFromStroage(uint64_t segmentId, uint32_t blockId);

    /**
     * Save a block to storage
     * @param blockData a BlockData structure
     * @return Length of block if success, -1 if failure
     */

    uint32_t saveBlockToStorage(BlockData blockData);

    /**
     * Perform degraded read of an segment
     * @param segmentId ID of the segment to read
     * @return an SegmentData structure
     */

    struct SegmentData degradedRead(uint64_t segmentId);

    /**
     * Free segmentData
     * @param segmentId Segment ID
     * @param segmentData Segment Data structure
     */

    void freeSegment(uint64_t segmentId, SegmentData segmentData);
    void sendBlock(uint32_t sockfd, struct BlockData blockData,
                       DataMsgType dataMsgType, string updateKey = "");
    /**
     * Stores the list of CHUNKSERVERs that store a certain block
     */

    //BlockLocationCache* _blockLocationCache;
    /**
     * Handles communication with other components
     */

    ChunkserverCommunicator* _chunkserverCommunicator;

    /**
     * Handles the storage layer
     */

    StorageModule* _storageModule;

    /**
     * Handles coding and decoding
     */

    CodingModule* _codingModule;

    //	Coding _cunit; // encode & decode done here
    uint32_t _chunkserverId;

    // upload
    ConcurrentMap<uint64_t, uint32_t> _pendingSegmentChunk;
    ConcurrentMap<uint64_t, struct CodingSetting> _codingSettingMap;
    ConcurrentMap<string, BlockData> _uploadBlockData;

    // download
    ConcurrentMap<uint32_t, uint32_t> _blocktpRequestCount;
    atomic<uint32_t> _blocktpId;

    ConcurrentMap<uint64_t, vector<struct BlockData>> _downloadBlockData;
    ConcurrentMap<uint64_t, uint32_t> _downloadBlockRemaining;
    ConcurrentMap<uint64_t, uint32_t> _segmentRequestCount;
    ConcurrentMap<uint64_t, mutex*> _segmentDownloadMutex;
    ConcurrentMap<uint64_t, SegmentData> _segmentDataMap;
    ConcurrentMap<uint64_t, bool> _isSegmentDownloaded;

    // recovery
    ConcurrentMap<string, bool> _isPendingRecovery;
    ConcurrentMap<string, uint32_t> _pendingRecoveryBlockChunk;
    ConcurrentMap<string, BlockData> _recoveryBlockData;
    ConcurrentMap<uint32_t, uint32_t> _recoverytpRequestCount;
    ConcurrentMap<uint32_t, uint32_t> _recoverytpRequestCountx;
    ConcurrentMap<uint32_t, uint32_t> _deltatpRequestCount;

    ConcurrentMap<string, bool> _isPendingRecoveryDelta;
    ConcurrentMap<string, bool> _isOK;
    ConcurrentMap<string, uint32_t> _deltaChunk;
    ConcurrentMap<string, string> _ifFinishGetDelta;
    ConcurrentMap<string, uint32_t> _pendingRecoveryDeltaChunk;
    ConcurrentMap<string, uint32_t> _allDeltaSize;
    ConcurrentMap<string, BlockData> _recoveryDeltaData;
    atomic<uint32_t> _recoverytpId;

    // update
    ConcurrentMap<string, BlockData> _updateBlockData;
    ConcurrentMap<string, uint32_t> _pendingUpdateSegmentChunk;
    ConcurrentMap<string, uint32_t> _pendingUpdateBlockChunk;
    atomic<uint32_t> _updateId;

    // upload / download
    ConcurrentMap<string, uint32_t> _pendingBlockChunk;

    // cache report
    uint32_t _reportCacheInterval;
    list<uint64_t> _previousCacheList;

    vector<pair<uint64_t, uint32_t>> _latencyList; // <isUpdate, latency>

    uint32_t _updateScheme;
    uint64_t _reservedSpaceSize;

};
#endif
