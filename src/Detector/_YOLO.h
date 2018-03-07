/*
 * _YOLO.h
 *
 *  Created on: Mar 7, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__YOLO_H_
#define OpenKAI_src_Detector__YOLO_H_

#include "../Base/common.h"
#include "../Vision/_VisionBase.h"
#include "_DetectorBase.h"

#ifdef USE_DARKNET

extern "C" {
#include "api.h"
}

namespace kai
{

class _YOLO: public _DetectorBase
{
public:
	_YOLO();
	~_YOLO();

	bool init(void* pKiss);
	bool link(void);
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
	int m_nBox;
	int m_nBoxMax;
	uint32_t m_nClass;

	Frame* m_pRGBA;
	Frame* m_pRGBAf;

	double m_minSize;
	double m_maxSize;
	vDouble4 m_area;

	string m_className;

};

}

#endif
#endif
