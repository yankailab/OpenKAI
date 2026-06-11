/*
 * _VideoRecorder.h
 *
 *  Created on: June 11, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__VideoRecorder_H_
#define OpenKAI_src_Vision__VideoRecorder_H_

#include "_VisionBase.h"

namespace kai
{

	class _VideoRecorder : public _VisionBase
	{
	public:
		_VideoRecorder();
		virtual ~_VideoRecorder();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual void console(const json &j, void *pJSONbase);

	private:
		void recFrame(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_VideoRecorder *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV;

		vInt2 m_vSize;
		string m_gstOutput;
		VideoWriter m_gst;
	};

}
#endif
