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
#include "../../Protocol/_ProtocolBase.h"

#define PC_N_BUF 1024

namespace kai
{
	class _PCrecv : public _PCstream
	{
	public:
		_PCrecv();
		virtual ~_PCrecv();

		virtual bool init(const json &j);
		virtual bool link(const json& j, ModuleMgr* pM);

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);

	protected:
		virtual bool readCMD(PROTOCOL_CMD *pCmd);
		virtual void handleCMD(const PROTOCOL_CMD &cmd);
		virtual void decodeStream(const PROTOCOL_CMD &cmd);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_PCrecv *)This)->update();
			return NULL;
		}

	protected:
		_IObase *m_pIO;
		uint64_t m_nCMDrecv;

		uint8_t m_pBuf[PC_N_BUF];
		int m_nRead;
		int m_iRead;
	};

}
#endif
