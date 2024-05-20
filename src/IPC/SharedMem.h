/*
 * SharedMem.h
 *
 *  Created on: Sept 20, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_IPCSharedMemBase_H_
#define OpenKAI_src_IPCSharedMemBase_H_

#include "../Base/BASE.h"
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace kai
{

	class SharedMem : public BASE
	{
	public:
		SharedMem();
		virtual ~SharedMem();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool open(void);
		virtual bool bOpen(void);
		virtual void close(void);
		
		virtual int nB(void);
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
