/*
 * _FrameBuffer.h
 *
 *  Created on: May 24, 2022
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_FrameBuffer_H_
#define OpenKAI_src_UI_FrameBuffer_H_

#include "_UIbase.h"
#include "../Vision/Frame.h"
#include "../Primitive/tSwap.h"

namespace kai
{
	class _FrameBuffer : public _UIbase
	{
	public:
		_FrameBuffer();
		virtual ~_FrameBuffer();

		virtual bool init(void *pKiss);
		virtual bool start(void);

	protected:
		void updateFB(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_FrameBuffer *)This)->update();
			return NULL;
		}

	private:
		tSwap<Frame> m_sF;
		vInt2 m_vSize;

		string m_gstOutput;
		VideoWriter m_gst;
	};
}
#endif
