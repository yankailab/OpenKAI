/*
 * SharedMemoryFrame.h
 *
 *  Created on: Sept 20, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_DataObject_SharedMem_H_
#define OpenKAI_src_DataObject_SharedMem_H_

#include "DataObjBase.h"

#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace kai
{
	class SharedMemoryFrame : public DataObjBase
	{
	public:
		SharedMemoryFrame();
		virtual ~SharedMemoryFrame();

		virtual bool loadConfig(void) override;
		virtual bool saveConfig(bool bExport) override;



		virtual int nB(void);
		virtual void* p(void);
		virtual bool bWriter(void);

	protected:
		string m_shmName = "";
		int m_nB = 0;
		int m_fd = 0;
		void* m_pB = 0;
		bool m_bWriter = true;

		bool m_bOpened = false;

	};

}
#endif
