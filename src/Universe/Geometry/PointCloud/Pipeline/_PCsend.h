/*
 * _PCsend.h
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud_PCsend_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_PCsend_H_

#include "../../_GeometryBase.h"
#include "../../../../DataStream/PCLframe.h"
#include "../../../../IO/_IObase.h"
#include "PCstreamProtocol.h"

namespace kai
{
	class _PCsend : public _GeometryBase
	{
	public:
		_PCsend();
		virtual ~_PCsend();

		bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool link(InstanceMgr *pM) override;
		bool start(void) override;
		bool check(void) override;

	private:
		void sendPC(void);
		void update(void);
		static void *getUpdate(void *pThis)
		{
			static_cast<_PCsend *>(pThis)->update();
			return nullptr;
		}

	protected:
		_IObase *m_pIO = nullptr;
		PCLframe *m_pPCLin = nullptr;
		uint64_t m_inputRevision = 0;
		vector<uint8_t> m_vPacket;
		int m_nB = 2000;
		uint64_t m_tInt = NSEC_SEC / 10;
	};
}
#endif
