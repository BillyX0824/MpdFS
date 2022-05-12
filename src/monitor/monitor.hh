/* 
 * File system MDPFS
 * Copyright (c) 2022 Xiaosong Su 
 * All rights reserved.
 *
 */

#ifndef __MONITOR_HH__
#define __MONITOR_HH__

#include "monitor_communicator.hh"
#include "selectionmodule.hh"
#include "recoverymodule.hh"
#include "statmodule.hh"
#include "../common/onlinechunkserver.hh"
#include "../common/chunkserverstat.hh"
#include "../cache/cache.hh"
#include <map>
#include <mutex>

using namespace std;

class Monitor {

public:
	/**
	 * Constructor
	 */
	Monitor();

	/**
	 * Desctructor
	 */
	~Monitor();

	//getter functions

	/**
	 * Get a reference of MonitorCommunicator
	 * @return Pointer to Monitor communication module
	 */
	MonitorCommunicator* getCommunicator();

	/**
	 * Get a reference of Monitor's status module
	 * @return Pointer to statmodule
	 */
	StatModule* getStatModule();

	/**
	 * Get a reference of Monitor's recovery module
	 * @return Pointer to recoverymodule
	 */
	RecoveryModule* getRecoveryModule();

	/**
	 * Get the unique Monitor ID
	 * @return monitor id in uint32_t
	 */
	uint32_t getMonitorId();

	// Processors
	
	/**
	 * Action when an CHUNKSERVER startup message received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param chunkserverId CHUNKSERVER ID
	 * @param capacity Free space on the CHUNKSERVER
	 * @param loading Current CPU loading on the CHUNKSERVER
	 * @param ip CHUNKSERVER's ip address for other components to connect
	 * @param port CHUNKSERVER's port for other components to connect
	 */
	void ChunkserverStartupProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t chunkserverId, uint32_t capacity, uint32_t loading, uint32_t ip,
		uint16_t port);

	/**
	 * Action when an CHUNKSERVER update its status message received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param chunkserverId CHUNKSERVER ID
	 * @param capacity Free space on the CHUNKSERVER
	 * @param loading Current CPU loading on the CHUNKSERVER
	 */
	void ChunkserverStatUpdateReplyProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t chunkserverId, uint32_t capacity, uint32_t loading);
	
	/**
	 * Action when an CHUNKSERVER shutdown message received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param chunkserverId CHUNKSERVER ID
	 */
	void ChunkserverShutdownProcessor(uint32_t requestId, uint32_t sockfd, uint32_t chunkserverId);
	
	/**
	 * Action when an Master request Primary CHUNKSERVERs for upload file
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param numOfObjs The number of CHUNKSERVERs required 
	 */
	void getPrimaryListProcessor(uint32_t requestId, uint32_t sockfd, uint32_t numOfObjs);
	
	/**
	 * Action when an CHUNKSERVER request Secondary CHUNKSERVERs for coding
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param numOfSegs Number of CHUNKSERVERs required for coding
	 */
	void getSecondaryListProcessor (uint32_t requestId, uint32_t sockfd, uint32_t
		 numOfBlks, uint32_t primaryId, uint64_t blockSize);

	/**
	 * Action when a CLIENT request current ONLIE CHUNKSERVERs for connection
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 */
	void getChunkserverListProcessor (uint32_t requestId, uint32_t sockfd);

	/**
	 * Action when a CLIENT request current ONLIE CHUNKSERVERs for connection
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param chunkserverListRef request chunkserver id list reference 
	 */
	void getChunkserverStatusRequestProcessor (uint32_t requestId, uint32_t sockfd,
		vector<uint32_t>& chunkserverListRef);
	
	uint32_t getDeadPeriod();

	uint32_t getSleepPeriod();
	
	uint32_t getUpdatePeriod();

private:
	
	/**
	 * Handles communication with other components
	 */
	MonitorCommunicator* _monitorCommunicator;

	/**
	 * Handles selection when request need a number of CHUNKSERVERs to process jobs
	 */
	SelectionModule* _selectionModule;
	
	/**
	 * Handles recovery fairs of CHUNKSERVERs
	 */
	RecoveryModule* _recoveryModule;
	
	/**
	 * Manage all the CHUNKSERVER status update fairs on this module
	 */
	StatModule* _statModule;

	/**
	 * the map used to store all the chunkserver status
	 */	
	map<uint32_t, struct ChunkserverStat> _chunkserverStatMap;

	/**
	 * the map used in selection module for load balancing
	 */	
	map<uint32_t, struct ChunkserverLBStat> _chunkserverLBMap;


	/**
	 * Unique ID for this monitor
	 */
	uint32_t _monitorId;
	
	/**
	 * The period sleep time for recovery check
	 */
	uint32_t _sleepPeriod;

	/**
	 * The dead thershold
	 */
	uint32_t _deadPeriod;

	/**
	 * The period sleep time for stat update
	 */
	uint32_t _updatePeriod;

    /**
     * Mutex for CHUNKSERVER start up
     */
    mutex _chunkserverStartUpProcessorMutex;
};

#endif
