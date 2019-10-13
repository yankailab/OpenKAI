/*
 * _ArUco.h
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__ArUco_H_
#define OpenKAI_src_Detector__ArUco_H_

#include "../Base/common.h"
#include "../Detector/_DetectorBase.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

class _ArUco : public _DetectorBase
{
public:
	_ArUco();
	virtual ~_ArUco();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ArUco*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pVision;
	cv::Ptr<cv::aruco::Dictionary> m_pDict;

	uint8_t m_dict;

};

}
#endif
#endif
