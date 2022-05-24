/*
 * _FrameBuffer.h
 *
 *  Created on: May 24, 2022
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_FrameBuffer_H_
#define OpenKAI_src_UI_FrameBuffer_H_

#include "../Base/_ModuleBase.h"
#include "../Vision/Frame.h"
#include "../Primitive/tSwap.h"

namespace kai
{
	class _FrameBuffer : public _ModuleBase
	{
	public:
		_FrameBuffer();
		virtual ~_FrameBuffer();

		bool init(void *pKiss);
		bool start(void);

		Frame *getFrame(void);
		Frame *getNextFrame(void);

	protected:
		void updateFB(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_FrameBuffer *)This)->update();
			return NULL;
		}

	private:
		vector<BASE *> m_vpB;
		tSwap<Frame> m_sF;
		vInt2 m_vSize;

		string m_gstOutput;
		VideoWriter m_gst;

	};
}
#endif
