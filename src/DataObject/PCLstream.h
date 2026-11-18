#ifndef OpenKAI_src_DataObject_PCLstream_H_
#define OpenKAI_src_DataObject_PCLstream_H_

#include "DataObjStream.h"

namespace kai
{
	struct GEOMETRY_POINT
	{
		Vector3f m_vP = Vector3f::Zero();
		Vector3f m_vC = Vector3f::Zero();
		uint64_t m_tStamp = 0; // capture time in nanoseconds; zero is invalid

		void clear()
		{
			m_vP.setZero();
			m_vC.setZero();
			m_tStamp = 0;
		}
	};

	class PCLstream : public DataObjStream<GEOMETRY_POINT>
	{
	public:
		PCLstream();
		virtual ~PCLstream() = default;
		bool loadConfig() override;
		bool saveConfig(bool bExport) override;

		// Zero keeps the current capacity; all slots reset to invalid points.
		bool clear(size_t nBuf = 0);
		// The cursor is a point capture timestamp, not this object's update stamp.
		// Require it explicitly so PCLframe's whole-buffer get remains unambiguous.
		uint64_t get(vector<GEOMETRY_POINT> &dest, uint64_t tStampFrom);

	protected:
		size_t m_nBuf = 100000;
	};
}
#endif
