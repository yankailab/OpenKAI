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

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);

	private:
		virtual void update(void);
		bool update_SharedMemImg(void);
		static void *getUpdate(void *This)
		{
			((_SharedMemImg *)This)->update();
			return NULL;
		}

	public:
		SharedMem* m_pSHM;
		int m_matType;

	};

}
#endif
