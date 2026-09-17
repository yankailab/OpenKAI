/*
 * _Canvas.h
 *
 *  Created on: June 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Canvas__Canvas_H_
#define OpenKAI_src_Canvas__Canvas_H_

#include "_ObjectArray.h"
#include "../Primitive/tSwap.h"

#ifdef USE_OPENCV
#include "../Base/cv.h"
#endif

namespace kai
{
	class _Canvas : public _ModuleBase
	{
	public:
		_Canvas();
		virtual ~_Canvas();

		virtual bool init(const json& j);
		virtual bool start(void);
		virtual void update(void);
		virtual void draw(void *pMat);
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
			((_Canvas *)This)->update();
			return NULL;
		}

	protected:
		//general
		uint8_t m_nDim;

		//data
		tSwap<_ObjectArray> m_sO;

		//config
		float m_minConfidence = 0.0;
		Vector2f m_vArea = Vector2f::Zero();
		Vector2f m_vW = Vector2f::Zero();
		Vector2f m_vH = Vector2f::Zero();
		Vector4f m_vRoi = Vector4f::Zero();
		Vector2i m_vClassRange = Vector2i::Zero();

		//show
		bool m_bDrawText = false;
		bool m_bDrawPos = false;
		bool m_bDrawBB = false;
	};

}
#endif
