/*
 * _Gstreamer.h
 *
 *  Created on: Dec 21, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__GStreamer_H_
#define OpenKAI_src_Vision__GStreamer_H_

#include "_VisionBase.h"

namespace kai
{

	class _GStreamer : public _VisionBase
	{
	public:
		_GStreamer();
		virtual ~_GStreamer();

		bool init(void *pKiss);
		bool start(void);
		bool open(void);
		void close(void);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_GStreamer *)This)->update();
			return NULL;
		}

	public:
		string m_pipeline;
		VideoCapture m_gst;
		int m_nInitRead;
	};

}
#endif
