/*
 * _SurfaceBase.h
 *
 *  Created on: June 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Surface__SurfaceBase_H_
#define OpenKAI_src_Universe_Surface__SurfaceBase_H_

#include "../_ReferenceFrame.h"
#include "../Object/_ObjectBase.h"

#ifdef USE_OPENCV
#include "../../Base/cv.h"
#endif

namespace kai
{
	class _SurfaceBase : public _ReferenceFrame
	{
	public:
		_SurfaceBase();
		virtual ~_SurfaceBase();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;

		virtual bool start(void);
		virtual void update(void);
		virtual void draw(void *pMat);
		virtual void console(void *pConsole);

		//io
		virtual _ObjectBase *add(const _ObjectBase &pO);
		virtual _ObjectBase *get(int i);
		virtual int size(void);

		virtual void clear(void);

	private:
		static void *getUpdate(void *This)
		{
			((_SurfaceBase *)This)->update();
			return NULL;
		}

	protected:
		RingBuffer<_ObjectBase> m_rObj;
		


		//show
		bool m_bDrawText = false;
		bool m_bDrawPos = false;
		bool m_bDrawBB = false;
	};

}
#endif
