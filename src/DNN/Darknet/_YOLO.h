/*
 * _YOLO.h
 *
 *  Created on: Mar 7, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Darknet__YOLO_H_
#define OpenKAI_src_DNN_Darknet__YOLO_H_

#include "../../Base/common.h"
#include "../../Vision/_VisionBase.h"
#include "../../Base/_ObjectBase.h"

#ifdef USE_DARKNET

extern "C" {
#include "api.h"
}

namespace kai
{

class _YOLO: public _ObjectBase
{
public:
	_YOLO();
	~_YOLO();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_YOLO*) This)->update();
		return NULL;
	}

public:
	double m_thresh;
	double m_hier;
	double m_nms;
	int m_nPredAvr;

	yolo_object* m_pYoloObj;
	int m_nBatch;
	Frame m_BGR;
};

}

#endif
#endif
