/*
 * _SharedMem.h
 *
 *  Created on: Sept 20, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_IPC_SharedMemBase_H_
#define OpenKAI_src_IPC_SharedMemBase_H_

#include "../Base/_ModuleBase.h"
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace kai
{

	class _SharedMem : public _ModuleBase
	{
	public:
		_SharedMem();
		virtual ~_SharedMem();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool open(void);
		virtual bool bOpen(void);
		virtual void close(void);
		
		virtual void* p(void);
		virtual bool bWriter(void);

	public:
		string m_shmName;
		int m_nB;
		int m_fd;
		void* m_pB;
		bool m_bWriter;

		bool m_bOpen;
		
	};

}
#endif
