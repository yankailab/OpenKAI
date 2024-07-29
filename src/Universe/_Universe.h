/*
 * _Universe.h
 *
 *  Created on: June 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe__Universe_H_
#define OpenKAI_src_Universe__Universe_H_

#include "_ObjectArray.h"
#include "../Primitive/tSwap.h"

#ifdef USE_OPENCV
#include "../Vision/Frame.h"
#endif

namespace kai
{
	class _Universe : public _ModuleBase
	{
	public:
		_Universe();
		virtual ~_Universe();

		virtual int init(void *pKiss);
		virtual int start(void);
		virtual void update(void);
		virtual void draw(void *pFrame);
		virtual void console(void *pConsole);

		//io
		virtual _Object *add(_Object &pO);
		virtual _Object *get(int i);
		virtual int size(void);

		virtual void clear(void);
		virtual void swap(void);

	private:
		static void *getUpdate(void *This)
		{
			((_Universe *)This)->update();
			return NULL;
		}

	public:
		//general
		uint8_t m_nDim;

		//data
		tSwap<_ObjectArray> m_sO;

		//config
		float m_minConfidence;
		vFloat2 m_vArea;
		vFloat2 m_vW;
		vFloat2 m_vH;
		vFloat4 m_vRoi;
		vInt2 m_vClassRange;

		//show
		bool m_bDrawText;
		bool m_bDrawPos;
	};

}
#endif
