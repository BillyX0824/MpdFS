/*
 * chunkserver_main.cc
 */

#include <signal.h>
#include <thread>
#include <vector>
#include "chunkserver.hh"
#include <iostream>
#include <iomanip>
using namespace std;
#include "../common/blocklocation.hh"
#include "../common/debug.hh"
#include "../common/garbagecollector.hh"
#include "../common/netfunc.hh"
#include "../protocol/status/getchunkserverstatusrequestmsg.hh"
#include "../common/define.hh"
#include "../../lib/deathhandler/death_handler.h"

/// Chunkserver Segment
Chunkserver* chunkserver;

/// Config Segment
ConfigLayer* configLayer;

// handle ctrl-C for profiler
void sighandler(int signum) {
	cout << "Signal" << signum << "received" << endl;
	if (signum == SIGINT) {
		debug_yellow ("%s\n", "SIGINT received\n");
		exit(42);
	} else if (signum == SIGUSR1) {
		cout << "Dumping latency results...";
		fflush (stdout);
		chunkserver->dumpLatency();
		cout << "done" << endl;
	}
}

/**
 * Main function
 * @return 0 if success;
 */

int main(int argc, char* argv[]) {

	signal(SIGINT, sighandler);
	signal(SIGUSR1, sighandler);

	// handle segFault for debug
	//Debug::DeathHandler dh;
	//(void) dh; // avoid warning

	char* interfaceName = NULL;
	char* gwIP = NULL;
	bool forward = false;
	uint32_t selfId = 0;

	if (argc < 3) {
		cout << "Usage: ./CHUNKSERVER [ID] [NETWORK INTERFACE]" << endl;
		cout << "Usage: ./CHUNKSERVER [ID] FORWARD [GATEWAY IP]" << endl;
		exit(0);
	} else {
		selfId = atoi(argv[1]);
		interfaceName = argv[2];
		if (strcmp(interfaceName, "FORWARD") == 0) {
			forward = true;
			if (argc < 4) {
				cout << "Usage: ./CHUNKSERVER [ID] [NETWORK INTERFACE]" << endl;
				cout << "Usage: ./CHUNKSERVER [ID] FORWARD [GATEWAY IP]" << endl;
				exit(0);
			} else
				gwIP = argv[3];
		}
	}

	// create new CHUNKSERVER segment and communicator
	chunkserver = new Chunkserver(selfId);

	// create new communicator
	ChunkserverCommunicator* communicator = chunkserver->getCommunicator();

	if (forward)
		communicator->setServerPort((uint16_t)selfId);

	// set identity
	communicator->setId(chunkserver->getChunkserverId());
	communicator->setComponentType(CHUNKSERVER);

	// create server
	communicator->createServerSocket();

	// 1. Garbage Collection Thread (lamba function hack for singleton)
	thread garbageCollectionThread([&](){GarbageCollector::getInstance().start();});

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, communicator);

	uint32_t selfAddr;
	uint16_t selfPort;
	if (forward) {
		struct sockaddr_in addr;
		inet_pton(AF_INET, gwIP, &(addr.sin_addr));
		selfAddr = addr.sin_addr.s_addr;
	} else
		selfAddr = getInterfaceAddressV4(interfaceName);
	selfPort = communicator->getServerPort();

	printIp(selfAddr);
	printf("Port = %hu\n",selfPort);

	//communicator->connectAllComponents();
	communicator->connectToMyself("127.0.0.1", selfPort, CHUNKSERVER);
	//communicator->connectToMyself(Ipv4Int2Str(selfAddr), selfPort, CHUNKSERVER);
	communicator->connectToMaster();
	communicator->connectToMonitor();
	communicator->registerToMonitor(selfAddr, selfPort);

	garbageCollectionThread.join();
	receiveThread.join();

	// cleanup
	delete configLayer;
	delete chunkserver;

	return 0;
}




