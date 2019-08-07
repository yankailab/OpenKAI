/*
 * _ShopCam.h
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__ShopCam_H_
#define OpenKAI_src_Application__ShopCam_H_

#include "../../Base/common.h"
#include "../../Detector/_DetectorBase.h"
#include "../../Detector/_DNNclassifier.h"

namespace kai
{

class _ShopCam: public _DetectorBase
{
public:
	_ShopCam(void);
	virtual ~_ShopCam();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

private:
	bool updateDet(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ShopCam *) This)->update();
		return NULL;
	}

private:
	_DetectorBase* m_pD;
	_DNNclassifier* m_pG;
	_DNNclassifier* m_pA;

	Frame m_fBGR;

};

}
#endif
