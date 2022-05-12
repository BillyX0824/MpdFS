/**
 * listdirectoryrequest.cc
 */

#include <iostream>
#include "listdirectoryrequest.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/debug.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MASTER
#include "../../master/master.hh"
extern Master* master;
#endif

/**
 * Default Constructor
 */

ListDirectoryRequestMsg::ListDirectoryRequestMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
ListDirectoryRequestMsg::ListDirectoryRequestMsg(Communicator* communicator,
		uint32_t clientId, uint32_t masterSockfd, const string &path) :
		Message(communicator) {
	_clientId = clientId;
	_directoryPath = path;
	_sockfd = masterSockfd;
}

void ListDirectoryRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ListDirectoryRequestPro listDirectoryRequestPro;
	listDirectoryRequestPro.set_directorypath(_directoryPath);
	listDirectoryRequestPro.set_chunkserverid(_clientId);

	if (!listDirectoryRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(LIST_DIRECTORY_REQUEST);
	setProtocolMsg(serializedString);

}

void ListDirectoryRequestMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::ListDirectoryRequestPro listDirectoryRequestPro;
	listDirectoryRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = listDirectoryRequestPro.chunkserverid();
	_directoryPath = listDirectoryRequestPro.directorypath();

}

void ListDirectoryRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MASTER
	master->listFolderProcessor(_msgHeader.requestId,_sockfd,_clientId,_directoryPath);
#endif
}

void ListDirectoryRequestMsg::printProtocol() {
	debug("[LIST_DIRECTORY_REQUEST] CHUNKSERVER ID = %" PRIu32 " Path = %s\n",
			_clientId, _directoryPath.c_str());
}

/**
 * @brief	Set the Folder Data
 */
void ListDirectoryRequestMsg::setFolderData(vector<FileMetaData> folderData) {
	_folderData = folderData;

	return;
}

vector<FileMetaData> ListDirectoryRequestMsg::getFolderData() {
	return _folderData;
}
