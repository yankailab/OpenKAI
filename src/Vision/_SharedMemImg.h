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
#include "../IPC/_SharedMem.h"

namespace kai
{

	class _SharedMemImg : public _VisionBase
	{
	public:
		_SharedMemImg();
		virtual ~_SharedMemImg();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);

	private:
		void update(void);
		bool updateSharedMemImg(void);
		static void *getUpdate(void *This)
		{
			((_SharedMemImg *)This)->update();
			return NULL;
		}

	public:
		_SharedMem* m_pSHM;
		int m_matType;

	};

}
#endif
