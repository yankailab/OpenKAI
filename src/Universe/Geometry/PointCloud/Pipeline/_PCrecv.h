/*
 * _PCrecv.h
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud_PCrecv_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_PCrecv_H_

#include "../_PointCloud.h"
#include "../../../../IO/_IObase.h"
#include "PCstreamProtocol.h"

namespace kai
{
	class _PCrecv : public _PointCloud
	{
	public:
		_PCrecv();
		virtual ~_PCrecv();

		bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool link(InstanceMgr *pM) override;
		bool start(void) override;
		bool check(void) override;

	protected:
		void inputByte(uint8_t byte);
		void decodePacket(void);

	private:
		void update(void);
		static void *getUpdate(void *pThis)
		{
			static_cast<_PCrecv *>(pThis)->update();
			return nullptr;
		}

	protected:
		_IObase *m_pIO = nullptr;
		vector<uint8_t> m_vPacket;
		size_t m_nPacketBytes = 0;
		// Private decoder staging; only complete frames enter the DataStream.
		vector<GEOMETRY_POINT> m_vPendingPoints;
		uint64_t m_pendingRevision = 0;
		uint64_t m_tPendingStamp = 0;
		uint32_t m_nPendingPoints = 0;
		uint32_t m_nPmax = 2000000;
	};
}
#endif
