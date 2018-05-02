/*
 * _Gstreamer.h
 *
 *  Created on: Dec 21, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__GStreamer_H_
#define OpenKAI_src_Vision__GStreamer_H_

#include "../Base/common.h"
#include "_VisionBase.h"

namespace kai
{

class _GStreamer: public _VisionBase
{
public:
	_GStreamer();
	virtual ~_GStreamer();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	void reset(void);

private:
	bool open(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_GStreamer*) This)->update();
		return NULL;
	}

	void updateTPP(void);
	static void* getTPP(void* This)
	{
		((_GStreamer*) This)->updateTPP();
		return NULL;
	}

public:
	string m_pipeline;
	VideoCapture m_gst;

};

}

#endif
