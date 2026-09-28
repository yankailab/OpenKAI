#ifndef OpenKAI_src_DataStream_PCLmap_H_
#define OpenKAI_src_DataStream_PCLmap_H_

#include "DataStreamBase.h"
#include <memory>

namespace kai
{
	// A map keeps local submap points shared while loop closure changes their poses.
	class PCLmap : public DataStreamBase
	{
	public:
		struct Submap
		{
			uint64_t m_id = 0;
			uint64_t m_tStamp = 0;
			Isometry3d m_pose = Isometry3d::Identity();
			shared_ptr<const vector<Vector3f>> m_points;
		};

		struct Snapshot
		{
			uint64_t m_session = 0;
			uint64_t m_revision = 0;
			uint64_t m_tStamp = 0;
			vector<Submap> m_vSubmaps;
		};
		using SnapshotPtr = shared_ptr<const Snapshot>;

		PCLmap();
		SnapshotPtr get(void) const;
		void set(vector<Submap> submaps, uint64_t session, uint64_t tStamp = 0);

	private:
		SnapshotPtr m_snapshot;
		mutable std::shared_mutex m_sMutex;
	};
}
#endif
