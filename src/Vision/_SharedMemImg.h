/*
 * _SharedMemImg.h
 *
 *  Created on: Sept 20, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_SharedMemImg_H_
#define OpenKAI_src_Vision_SharedMemImg_H_

#include "_VisionBase.h"
#include "../Utility/util.h"
#include "../IPC/SharedMem.h"

namespace kai
{

	class _SharedMemImg : public _VisionBase
	{
	public:
		_SharedMemImg();
		virtual ~_SharedMemImg();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);

	private:
		virtual void update(void);
		bool updateSharedMemImg(void);
		static void *getUpdate(void *This)
		{
			((_SharedMemImg *)This)->update();
			return NULL;
		}

	protected:
		SharedMem *m_pSHM;
		int m_matType = CV_8UC3;
	};

}
#endif
