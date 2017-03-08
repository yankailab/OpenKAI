/*
 * _Cascade.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef AI__Cascade_H_
#define AI__Cascade_H_

#include "../Base/common.h"
#include "_AIbase.h"
#include "../Vision/_VisionBase.h"

namespace kai
{

class _Cascade: public _AIbase
{
public:
	_Cascade();
	~_Cascade();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

private:
	void detect(void);
	void addOrUpdate(OBJECT* pNewObj);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Cascade*) This)->update();
		return NULL;
	}

public:
	cv::CascadeClassifier	m_CC;
//	Ptr<cuda::CascadeClassifier> m_pCascade;

	double m_minSize;
	double m_maxSize;
	double m_overlapMin;
	vDouble4 m_area;

	string m_className;

};

}

#endif
