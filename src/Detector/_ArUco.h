/*
 * _ArUco.h
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__ArUco_H_
#define OpenKAI_src_Detector__ArUco_H_

#include "../Base/common.h"
#include "../Base/_ObjectBase.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

class _ArUco : public _ObjectBase
{
public:
	_ArUco();
	virtual ~_ArUco();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);
	bool cli(int& iY);

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
    std::vector<int> m_vID;
    std::vector<std::vector<cv::Point2f> > m_vvCorner;

	uint8_t m_dict;

};

}

#endif
#endif
