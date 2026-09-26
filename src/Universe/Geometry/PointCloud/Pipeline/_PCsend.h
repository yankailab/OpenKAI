/*
 * _PCsend.h
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud_PCsend_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_PCsend_H_

#include "../../../../Base/common.h"
#include "../../../../IO/_IObase.h"
#include "../../../../Protocol/_ProtocolBase.h"
#include "../../_GeometryBase.h"

#define PC_N_HDR 4
#define PC_STREAM 0
#define PC_FRAME_END 1

namespace kai
{

	class _PCsend : public _GeometryBase
	{
	public:
		_PCsend();
		virtual ~_PCsend();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);

	private:
		void sendPC(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCsend *)This)->update();
			return NULL;
		}

	protected:
		_IObase *m_pIO = nullptr;

		int m_iPsent = 0;

		uint8_t *m_pB = nullptr;
		int m_nB = 256;
		uint64_t m_tInt = NSEC_SEC / 10; // nanoseconds
	};

}
#endif
