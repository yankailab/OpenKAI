/*
 * _HiphenRGB.h
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__HiphenRGB_H_
#define OpenKAI_src_Application__HiphenRGB_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Navigation/_GPS.h"
#include "../../Vision/_Camera.h"
#include "../../Vision/Hiphen/_HiphenServer.h"

namespace kai
{

class _HiphenRGB: public _ThreadBase
{
public:
	_HiphenRGB(void);
	virtual ~_HiphenRGB();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	bool console(int& iY);
	int check(void);

private:
	void take(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_HiphenRGB *) This)->update();
		return NULL;
	}

private:
	_GPS* m_pGPS;
	_Camera* m_pCam;
	_HiphenServer* m_pHiphen;

	bool m_bFlip;
	vector<int> m_compress;
	int m_quality;

	int m_iImg;
	LL_POS m_LL;

};

}
#endif
