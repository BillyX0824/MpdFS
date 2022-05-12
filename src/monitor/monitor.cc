#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <signal.h>
#include "monitor.hh"
#include "../config/config.hh"
#include "../common/blocklocation.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"

ConfigLayer* configLayer;

Monitor* monitor;

mutex chunkserverStatMapMutex;
mutex chunkserverLBMapMutex;

void sighandler(int signum) {
	cout << "Signal" << signum << "received" << endl;
	if (signum == SIGUSR1) {
		cout << "Try to recover failure" << endl;
		monitor->getRecoveryModule()->userTriggerDetection();
		cout << "done" << endl;
	}else if (signum == SIGUSR2) {
		cout << "Try to recover failure with destination specified" << endl;
		monitor->getRecoveryModule()->userTriggerDetection(true);
		cout << "done" << endl;
	}
}

/*  Monitor default constructor
 */
Monitor::Monitor() {
	srand(time(NULL));

	_chunkserverStatMap = {};

	configLayer = new ConfigLayer("monitorconfig.xml");
	_monitorCommunicator = new MonitorCommunicator();
	_selectionModule = new SelectionModule(_chunkserverStatMap, _chunkserverLBMap);
	_statModule = new StatModule(_chunkserverStatMap);
	_recoveryModule = new RecoveryModule(_chunkserverStatMap, _monitorCommunicator);
	_monitorId = configLayer->getConfigInt("MonitorId");
	_sleepPeriod = configLayer->getConfigInt("SleepPeriod");
	_deadPeriod = configLayer->getConfigInt("DeadPeriod");
	_updatePeriod = configLayer->getConfigInt("UpdatePeriod");
}

/*	Monitor default desctructor
 */
Monitor::~Monitor() {
	delete _selectionModule;
	delete _recoveryModule;
	delete _statModule;
	delete _monitorCommunicator;
}

MonitorCommunicator* Monitor::getCommunicator() {
	return _monitorCommunicator;
}

StatModule* Monitor::getStatModule() {
	return _statModule;
}

RecoveryModule* Monitor::getRecoveryModule() {
	return _recoveryModule;
}

uint32_t Monitor::getMonitorId() {
	return _monitorId;
}

void Monitor::ChunkserverStartupProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t chunkserverId, uint32_t capacity, uint32_t loading, uint32_t ip,
		uint16_t port) {

    lock_guard<mutex> lk(_chunkserverStartUpProcessorMutex);

	debug(
			"CHUNKSERVER Startup Processor: on id = %" PRIu32 " ip = %" PRIu32 " port = %" PRIu32 "\n",
			chunkserverId, ip, port);
	// Send online chunkserver list to the newly startup chunkserver
	vector<struct OnlineChunkserver> onlineChunkserverList;

	_statModule->getOnlineChunkserverList(onlineChunkserverList);
	_monitorCommunicator->sendOnlineChunkserverList(sockfd, onlineChunkserverList);

	// Send the newly startup chunkserver stat to online chunkserver
	_statModule->broadcastNewChunkserver(_monitorCommunicator, chunkserverId, ip, port);

	// Add the newly startup chunkserver to the map
	_statModule->setStatById(chunkserverId, sockfd, capacity, loading, ONLINE, ip,
			port);

	// Add the newly startup chunkserver to load balancing map
	_selectionModule->addNewChunkserverToLBMap(chunkserverId);
}

void Monitor::ChunkserverStatUpdateReplyProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t chunkserverId, uint32_t capacity, uint32_t loading) {
	_statModule->setStatById(chunkserverId, sockfd, capacity, loading, ONLINE);
}

void Monitor::ChunkserverShutdownProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t chunkserverId) {
	_statModule->removeStatById(chunkserverId);
}

void Monitor::getPrimaryListProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t numOfObjs) {
	vector<uint32_t> primaryList;
	primaryList = _selectionModule->choosePrimary(numOfObjs);
	_monitorCommunicator->replyPrimaryList(requestId, sockfd, primaryList);
	return;
}

void Monitor::getSecondaryListProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t numOfBlks, uint32_t primaryId, uint64_t blockSize) {

	vector<struct BlockLocation> secondaryList;
	secondaryList = _selectionModule->chooseSecondary(numOfBlks, primaryId, blockSize);
	_monitorCommunicator->replySecondaryList(requestId, sockfd, secondaryList);
	return;
}

void Monitor::getChunkserverListProcessor(uint32_t requestId, uint32_t sockfd) {
	vector<struct OnlineChunkserver> chunkserverList;
	_statModule->getOnlineChunkserverList(chunkserverList);
	_monitorCommunicator->replyChunkserverList(requestId, sockfd, chunkserverList);
}

void Monitor::getChunkserverStatusRequestProcessor(uint32_t requestId, uint32_t sockfd,
		vector<uint32_t>& chunkserverListRef) {
	vector<bool> chunkserverStatus;
	_statModule->getChunkserverStatus(chunkserverListRef, chunkserverStatus);
	_monitorCommunicator->replyGetChunkserverStatus(requestId, sockfd, chunkserverStatus);
}

uint32_t Monitor::getDeadPeriod() {
	return _deadPeriod;
}

uint32_t Monitor::getSleepPeriod() {
	return _sleepPeriod;
}

uint32_t Monitor::getUpdatePeriod() {
	return _updatePeriod;
}

int main(void) {
	signal(SIGUSR1, sighandler);
	signal(SIGUSR2, sighandler);

	printf("MONITOR\n");

	monitor = new Monitor();
	MonitorCommunicator* communicator = monitor->getCommunicator();
	StatModule* statmodule = monitor->getStatModule();
#ifdef TRIGGER_RECOVERY
	RecoveryModule* recoverymodule = monitor->getRecoveryModule();
#endif

	// set up communicator
	communicator->setId(monitor->getMonitorId());
	communicator->setComponentType(MONITOR);
	communicator->createServerSocket();

	// 1. Garbage Collection Thread (lamba function hack for singleton)
	thread garbageCollectionThread(
			[&]() {GarbageCollector::getInstance().start();});

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, communicator);


	// 4. Update Thread
	thread updateThread(&StatModule::updateChunkserverStatMap, statmodule, communicator,
			monitor->getUpdatePeriod());

	// 5. Recovery Thread
#ifdef TRIGGER_RECOVERY
	thread recoveryThread(&RecoveryModule::failureDetection, recoverymodule,
			monitor->getDeadPeriod(), monitor->getSleepPeriod());
#endif

	// threads join
	garbageCollectionThread.join();
	receiveThread.join();
	updateThread.join();

#ifdef TRIGGER_RECOVERY
	recoveryThread.join();
#endif
	//clean up
	delete configLayer;
	delete monitor;

	return 0;
}
