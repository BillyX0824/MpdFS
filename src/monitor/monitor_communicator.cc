#include <iostream>
#include <thread>
#include <cstdio>
#include "monitor_communicator.hh"
#include "../config/config.hh"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../common/debug.hh"
#include "../protocol/nodelist/getprimarylistreply.hh"
#include "../protocol/nodelist/getsecondarylistreply.hh"
#include "../protocol/status/onlinechunkserverlistmsg.hh"
#include "../protocol/nodelist/getchunkserverlistreply.hh"
#include "../protocol/status/getchunkserverstatusreplymsg.hh"

using namespace std;

extern ConfigLayer* configLayer;

/**
 * Constructor
 */

MonitorCommunicator::MonitorCommunicator() {
	_serverPort = configLayer->getConfigInt("Communication>ServerPort");
}

/**
 * Destructor
 */

MonitorCommunicator::~MonitorCommunicator() {

}

void MonitorCommunicator::replyPrimaryList(uint32_t requestId, uint32_t sockfd, vector<uint32_t> primaryList){
	GetPrimaryListReplyMsg* getPrimaryListReplyMsg = new GetPrimaryListReplyMsg(this, requestId, sockfd, primaryList);
	getPrimaryListReplyMsg->prepareProtocolMsg();

	addMessage(getPrimaryListReplyMsg);
	return;
}

void MonitorCommunicator::replySecondaryList(uint32_t requestId, uint32_t sockfd, vector<struct BlockLocation> secondaryList){
	GetSecondaryListReplyMsg* getSecondaryListReplyMsg = new GetSecondaryListReplyMsg(this, requestId, sockfd, secondaryList);
	getSecondaryListReplyMsg->prepareProtocolMsg();

	addMessage(getSecondaryListReplyMsg);
	return;
}

void MonitorCommunicator::sendOnlineChunkserverList(uint32_t newChunkserverSockfd, 
		vector<struct OnlineChunkserver>& onlineChunkserverList) {
	OnlineChunkserverListMsg* onlineChunkserverListMsg = new OnlineChunkserverListMsg(this, newChunkserverSockfd, onlineChunkserverList);
	onlineChunkserverListMsg->prepareProtocolMsg();
	addMessage(onlineChunkserverListMsg);
}

void MonitorCommunicator::replyChunkserverList(uint32_t requestId, uint32_t clientSockfd, 
		vector<struct OnlineChunkserver>& onlineChunkserverList) {
	GetChunkserverListReplyMsg* getChunkserverListReplyMsg = new GetChunkserverListReplyMsg(this, requestId, 
		clientSockfd, onlineChunkserverList);
	getChunkserverListReplyMsg->prepareProtocolMsg();
	addMessage(getChunkserverListReplyMsg);
}

void MonitorCommunicator::replyGetChunkserverStatus(uint32_t requestId, uint32_t clientSockfd, 
		vector<bool>& chunkserverStatRef) {
	GetChunkserverStatusReplyMsg* getChunkserverStatusReplyMsg = new GetChunkserverStatusReplyMsg(this, requestId, 
		clientSockfd, chunkserverStatRef);
	getChunkserverStatusReplyMsg->prepareProtocolMsg();
	addMessage(getChunkserverStatusReplyMsg);
}
