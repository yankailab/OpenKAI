/*
 * _PCsend.h
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCsend_H_
#define OpenKAI_src_3D_PointCloud_PCsend_H_

#include "../../Base/common.h"
#include "../../IO/_IObase.h"
#include "../../Protocol/_ProtocolBase.h"
#include "../_GeometryBase.h"

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

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);

	private:
		void sendPC(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_PCsend *)This)->update();
			return NULL;
		}

	protected:
		_IObase *m_pIO;

		int m_iPsent;

		uint8_t *m_pB;
		int m_nB;
		uint64_t m_tInt;
	};

}
#endif
