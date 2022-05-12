#include <ctime>
#include <stdio.h>
#include <mutex>
#include <unistd.h>
#include <vector>
#include <set>
#include <thread>
#include <algorithm>
#include "recoverymodule.hh"
#include "../protocol/status/recoverytriggerrequest.hh"
#include "../protocol/status/repairsegmentinfomsg.hh"
using namespace std;

RecoveryModule::RecoveryModule(map<uint32_t, struct ChunkserverStat>& mapRef,
		MonitorCommunicator* communicator):
	_chunkserverStatMap(mapRef), _communicator(communicator) { 

	}

void startRecoveryProcedure(RecoveryModule* rm, vector<uint32_t> deadChunkserverList,
		bool dstSpecified, vector<uint32_t> dstSpec) {
	string chunkserverListString;
	for (auto chunkserverId : deadChunkserverList) {
		chunkserverListString += to_string(chunkserverId) + " ";
	}
	cout << "Start to recover failure CHUNKSERVER " << chunkserverListString << endl;
	rm->executeRecovery(deadChunkserverList, dstSpecified, dstSpec);
}


// Just sequentially choose one.
// Can use different startegy later.
void RecoveryModule::replaceFailedChunkserver(struct SegmentLocation& ol,
		struct SegmentRepairInfo& ret, map<uint32_t, uint32_t>& mapped) {

	lock_guard<mutex> lk(chunkserverStatMapMutex);
	ret.segmentId = ol.segmentId;

	vector<uint32_t>& ref = ol.chunkserverList;
	for (int pos = 0; pos < (int)ref.size(); ++pos) {
		if (_chunkserverStatMap[ref[pos]].chunkserverHealth != ONLINE) {
			uint32_t deadChunkserver = ref[pos];
			// mapped this dead chunkserver with new replacement
			ref[pos] = mapped[deadChunkserver];
			ret.repPos.push_back(pos);
			ret.repChunkserver.push_back(mapped[deadChunkserver]);
		}
	}

}
void RecoveryModule::replaceFailedChunkserver(struct SegmentLocation& ol, struct SegmentRepairInfo& ret) {
	// if not specify the dst, just sequential assign
	set<uint32_t> used;
	for (uint32_t chunkserverid : ol.chunkserverList) {
		if (_chunkserverStatMap[chunkserverid].chunkserverHealth == ONLINE) {
			used.insert(chunkserverid);
		}
	}

	set<uint32_t> avail;
	set<uint32_t> all;

	{
		lock_guard<mutex> lk(chunkserverStatMapMutex);
		for (auto& entry : _chunkserverStatMap) {
			if (entry.second.chunkserverHealth == ONLINE) {
				all.insert(entry.first);
				if (!used.count(entry.first))
					avail.insert(entry.first);
			}
		}

		set<uint32_t>::iterator it = avail.begin();

		ret.segmentId = ol.segmentId;

		vector<uint32_t>& ref = ol.chunkserverList;
		for (int pos = 0; pos < (int)ref.size(); ++pos) {
			if (_chunkserverStatMap[ref[pos]].chunkserverHealth != ONLINE) {
				if (it == avail.end() || it == all.end()) it = all.begin();
				if (it != avail.end() && it != all.end()) {
					debug_cyan("Faild chunkserver = %" PRIu32 "\n", ref[pos]);
					ref[pos] = *it;
					ret.repPos.push_back(pos);
					ret.repChunkserver.push_back(*it);
					debug_cyan("Replaced with chunkserver = %" PRIu32 "\n", ref[pos]);
					it++;
				} else {
					debug_cyan("%s\n", "[ERROR]: Failed to replace chunkserver");
				}
			}
		}
	}
}

void RecoveryModule::executeRecovery(vector<uint32_t>& deadChunkserverList, bool
		dstSpecified, vector<uint32_t> dstSpec) {
	// this map used if dstSpecified
	map<uint32_t, uint32_t> mapped;
	if (dstSpecified) {
		// one to one map from dead to dst
		sort(deadChunkserverList.begin(), deadChunkserverList.end());
		sort(dstSpec.begin(), dstSpec.end());
		for (int i = 0; i < (int)deadChunkserverList.size(); i++) {
			mapped[deadChunkserverList[i]] = dstSpec[i%dstSpec.size()];
		}
	}

	debug_yellow("%s\n", "Start Recovery Procedure");

	// Request Recovery to Master
	RecoveryTriggerRequestMsg* rtrm = new
		RecoveryTriggerRequestMsg(_communicator, _communicator->getMasterSockfd(),
				deadChunkserverList, dstSpecified, dstSpec); // add two fields dstSpec
	rtrm->prepareProtocolMsg();
	_communicator->addMessage(rtrm, true);

	MessageStatus status = rtrm->waitForStatusChange();
	if (status == READY) {
		vector<struct SegmentLocation> ols = rtrm->getSegmentLocations();
		for (struct SegmentLocation ol: ols) {
			// Print debug message
			debug_cyan("Segment Location id = %" PRIu64 " primary = %" PRIu32 "\n", ol.segmentId, 
					ol.primaryId);
			
			struct SegmentRepairInfo ori;
			if (dstSpecified) {
				replaceFailedChunkserver (ol, ori, mapped);	
			} else {
				replaceFailedChunkserver (ol, ori);	
			}
			ori.out();

			RepairSegmentInfoMsg* roim = new RepairSegmentInfoMsg(_communicator,
					_communicator->getSockfdFromId(ol.primaryId), ori.segmentId,
					ori.repPos, ori.repChunkserver);
			debug ("sockfd for repair = %" PRIu32 "\n", _communicator->getSockfdFromId(ol.primaryId));
			roim->prepareProtocolMsg();
			debug ("%s\n", "add repair segment info msg");
			_communicator->addMessage(roim);
			debug ("%s\n", "added to queue repair segment info msg");
		}

	} else {
		debug("%s\n", "Faided Recovery");
	}

}

void RecoveryModule::failureDetection(uint32_t deadPeriod, uint32_t sleepPeriod) {
	while (1) {
		{
			lock_guard<mutex> lk(triggerRecoveryMutex);
			uint32_t currentTS = time(NULL);
			vector<uint32_t> deadChunkserverList;
			{
				lock_guard<mutex> lk(chunkserverStatMapMutex);
				for(auto& entry: _chunkserverStatMap) {
					//entry.second.out();
					if ((currentTS - entry.second.timestamp) > deadPeriod &&
							(entry.second.chunkserverHealth != RECOVERING &&
							 entry.second.chunkserverHealth != ONLINE)) {
						// Trigger recovery 
						debug_yellow("Detect failure CHUNKSERVER = %" PRIu32 "\n", entry.first);
						deadChunkserverList.push_back (entry.first);
						entry.second.chunkserverHealth = RECOVERING;
					}
				}
			}
			// If has dead Chunkserver, start recovery
			if (deadChunkserverList.size() > 0) {
				thread recoveryProcedure(startRecoveryProcedure, this, deadChunkserverList, false, vector<uint32_t>());
				recoveryProcedure.detach();
			}
		}
		sleep(sleepPeriod);
	}
}

void RecoveryModule::userTriggerDetection(bool dstSpecified) {
	lock_guard<mutex> lk(triggerRecoveryMutex);
	vector<uint32_t> deadChunkserverList;
	{
		lock_guard<mutex> lk(chunkserverStatMapMutex);
		for(auto& entry: _chunkserverStatMap) {
			//entry.second.out();
			if ((entry.second.chunkserverHealth != RECOVERING &&
						entry.second.chunkserverHealth != ONLINE)) {
				// Trigger recovery 
				debug_yellow("Detect failure CHUNKSERVER = %" PRIu32 "\n", entry.first);
				deadChunkserverList.push_back (entry.first);
				entry.second.chunkserverHealth = RECOVERING;
			}
		}
	}
	// If has dead Chunkserver, start recovery
	if (deadChunkserverList.size() > 0) {
		vector<uint32_t> dstSpec;
		if (dstSpecified) {
			FILE* fp = fopen(RECOVERY_DST, "r");
			int chunkserverid; 
			while (fscanf(fp, "%d", &chunkserverid) != EOF) {
				dstSpec.push_back(chunkserverid);
			}
		}
		thread recoveryProcedure(startRecoveryProcedure, this, deadChunkserverList,
				dstSpecified, dstSpec);
		recoveryProcedure.detach();
	}
}

