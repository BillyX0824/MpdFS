#ifndef __SELECTIONMODULE_HH__
#define __SELECTIONMODULE_HH__

#include <stdint.h>
#include <vector>
#include <map>
#include "../common/chunkserverstat.hh"
#include "../common/blocklocation.hh"
#include "../protocol/status/chunkserverstatupdaterequestmsg.hh"

using namespace std;

class SelectionModule {

	public:

		/**
		 * Constructor
		 * @param mapRef Reference of the chunkserver status map in monitor class
		 */
		SelectionModule(map<uint32_t, struct ChunkserverStat>& mapRef, map<uint32_t, struct
				ChunkserverLBStat>& lbRef);

		/**
		 * Choose primary chunkservers from the chunkserver status map
		 * @param numOfSegs Number of CHUNKSERVERs going to be selected
		 * @return a list of selected chunkserver IDs  
		 */
		vector<uint32_t> choosePrimary(uint32_t numOfSegs);


		/**
		 * Choose secondary chunkservers from the chunkserver status map to store coded blocks
		 * @param numOfBlks Number of CHUNKSERVERs going to be selected
		 * @param primary Primary CHUNKSERVER id for this segment
		 * @param blkSize Each blk size of the encoded segment
		 * @return a list of selected chunkserver IDs  
		 */
		vector<struct BlockLocation> chooseSecondary(uint32_t numOfBlks, uint32_t
				primary, uint64_t blkSize);

		/**
		 * Add a newly startup chunkserver to the load balancing map
		 * @param chunkserverId Newly startup Chunkserver Id
		 */
		void addNewChunkserverToLBMap(uint32_t chunkserverId);

	private:

		/**
		 * References of the maps defined in the monitor class 
		 */
		map<uint32_t, struct ChunkserverStat>& _chunkserverStatMap;
		map<uint32_t, struct ChunkserverLBStat>& _chunkserverLBMap;

};

#endif
