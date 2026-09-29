#ifndef OpenKAI_src_DataStream_PCLmap_H_
#define OpenKAI_src_DataStream_PCLmap_H_

#include "DataObjBase.h"

namespace kai
{
	class PCLmap : public DataObjBase
	{
	public:
		struct Submap
		{
			uint64_t m_id = 0;
			uint64_t m_tStamp = 0;
			Isometry3d m_pose = Isometry3d::Identity();
			vector<Vector3f> m_points;
		};

		PCLmap();
		void set(const vector<Submap> &src, uint64_t session, uint64_t tStamp = 0);
		uint64_t get(vector<Submap> &dest, uint64_t &session);

	private:
		vector<Submap> m_vSubmaps;
		uint64_t m_session = 0;
		std::shared_mutex m_sMutex;
	};
}
#endif
