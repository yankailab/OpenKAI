/*
 * _PCsend.h
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud_PCsend_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_PCsend_H_

#include "../../../_ReferenceFrame.h"
#include "../../../../DataObject/PCLframe.h"
#include "../../../../DataObject/BytePacketStream.h"
#include "PCstreamProtocol.h"

namespace kai
{
	class _PCsend : public _ReferenceFrame
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
		BytePacketStream *m_pBpStreamOut = nullptr;
		PCLframe *m_pPCLin = nullptr;
		uint64_t m_tInput = 0;
		vector<GEOMETRY_POINT> m_vPoints;
		vector<uint8_t> m_vPacket;
		int m_nB = 2000;
		uint64_t m_tInt = NSEC_SEC / 10;
	};
}
#endif
