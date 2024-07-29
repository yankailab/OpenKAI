/*
 * _PCrecv.h
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCrecv_H_
#define OpenKAI_src_3D_PointCloud_PCrecv_H_

#include "_PCsend.h"
#include "_PCstream.h"
#include "../../IO/_IObase.h"
#include "../../Protocol/_ProtocolBase.h"

namespace kai
{

	class _PCrecv : public _PCstream
	{
	public:
		_PCrecv();
		virtual ~_PCrecv();

		int init(void *pKiss);
		int start(void);
		int check(void);

	private:
		virtual bool readCMD(void);
		virtual void handleCMD(void);
		void decodeStream(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_PCrecv *)This)->update();
			return NULL;
		}

	public:
		_IObase *m_pIO;
		PROTOCOL_CMD m_recvMsg;
		uint64_t m_nCMDrecv;
	};

}
#endif
