#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "repairsegmentinfomsg.hh"

#ifdef COMPILE_FOR_CHUNKSERVER
#include "../../chunkserver/chunkserver.hh"
extern Chunkserver* chunkserver;
#endif

#ifdef COMPILE_FOR_MASTER
#include "../../master/master.hh"
extern Master* master;
#endif

RepairSegmentInfoMsg::RepairSegmentInfoMsg(Communicator* communicator) :
		Message(communicator) {

}

RepairSegmentInfoMsg::RepairSegmentInfoMsg(Communicator* communicator,
		uint32_t sockfd, uint64_t segmentId, vector<uint32_t> deadBlockIds,
		vector<uint32_t> newChunkserverIds) :
		Message(communicator) {

	_sockfd = sockfd;
	_segmentId = segmentId;
	_deadBlockIds = deadBlockIds;
	_newChunkserverIds = newChunkserverIds;
}

void RepairSegmentInfoMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::RepairSegmentInfoPro repairSegmentInfoPro;

	repairSegmentInfoPro.set_segmentid(_segmentId);

	for (uint32_t sid : _deadBlockIds) {
		repairSegmentInfoPro.add_deadblockids(sid);
	}

	for (uint32_t oid : _newChunkserverIds) {
		repairSegmentInfoPro.add_newchunkserverids(oid);
	}

	if (!repairSegmentInfoPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(REPAIR_SEGMENT_INFO);
	setProtocolMsg(serializedString);

}

void RepairSegmentInfoMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::RepairSegmentInfoPro repairSegmentInfoPro;
	repairSegmentInfoPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = repairSegmentInfoPro.segmentid();

	_deadBlockIds.clear();
	for (int i = 0; i < repairSegmentInfoPro.deadblockids_size(); ++i) {
		_deadBlockIds.push_back(repairSegmentInfoPro.deadblockids(i));
	}

	_newChunkserverIds.clear();
	for (int i = 0; i < repairSegmentInfoPro.newchunkserverids_size(); ++i) {
		_newChunkserverIds.push_back(repairSegmentInfoPro.newchunkserverids(i));
	}

}

void RepairSegmentInfoMsg::doHandle() {
#ifdef COMPILE_FOR_MASTER
	master->repairSegmentInfoProcessor(_msgHeader.requestId, _sockfd, _segmentId, _deadBlockIds, _newChunkserverIds);
#endif
#ifdef COMPILE_FOR_CHUNKSERVER
	chunkserver->repairSegmentInfoProcessor(_msgHeader.requestId, _sockfd, _segmentId, _deadBlockIds, _newChunkserverIds);
#endif
}

void RepairSegmentInfoMsg::printProtocol() {
	debug("[REPAIR_SEGMENT_INFO]: %" PRIu64 "\n", _segmentId);
	for (int i = 0; i < (int) _deadBlockIds.size(); i++)
		debug("[REPAIR_SEGMENT_INFO]: (%" PRIu32 ", %" PRIu32 ")\n",
				_deadBlockIds[i], _newChunkserverIds[i]);
}
