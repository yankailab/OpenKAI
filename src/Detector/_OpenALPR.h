/*
 * _OpenALPR.h
 *
 *  Created on: March 19, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Detector__OpenALPR_H_
#define OpenKAI_src_DNN_Detector__OpenALPR_H_

#include "../Vision/_VisionBase.h"

#ifdef USE_OPENCV
#ifdef USE_OPENALPR
#include "_DNNtext.h"
#include <alpr.h>

namespace kai
{

class _OpenALPR: public _DetectorBase
{
public:
	_OpenALPR();
	~_OpenALPR();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	int check(void);

private:
	bool detect(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_OpenALPR*) This)->update();
		return NULL;
	}

public:
	alpr::Alpr* m_pAlpr;
	string	m_region;
	string	m_config;
	string	m_runtime;
	int		m_nTop;
	string	m_locale;

};

}
#endif
#endif
#endif
