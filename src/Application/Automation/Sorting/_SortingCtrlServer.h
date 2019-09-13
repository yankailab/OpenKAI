/*
 * _SortingImgServer.h
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Automation__SortingImgServer_H_
#define OpenKAI_src_Automation__SortingImgServer_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../../Protocol/_OKlink.h"

//0 OKLINK_BEGIN
//1 COMMAND
//2 PAYLOAD LENGTH
//3-6 ID
//7-8 iClass
//9-10 bb.x
//11-12 bb.y
//13-14 bb.z
//15-16 bb.w

#define OKLINK_BB 4
#define OKLINK_SETSTATE 5

#define SORT_STATE_OFF 0
#define SORT_STATE_ON 1

namespace kai
{

class _SortingCtrlServer: public _DetectorBase
{
public:
	_SortingCtrlServer(void);
	virtual ~_SortingCtrlServer();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

	void handleCMD(uint8_t* pCMD);
	static void callbackCMD(uint8_t* pCMD, void* pInst)
	{
		NULL_(pInst);
		_SortingCtrlServer* pS = (_SortingCtrlServer*)pInst;
		pS->handleCMD(pCMD);
	}

private:
	void updateImg(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SortingCtrlServer *) This)->update();
		return NULL;
	}

public:
	_DepthVisionBase* m_pDV;
	_OKlink*		m_pOL;
	int				m_iState;
	uint64_t		m_tLastSentState;

	int				m_ID;
	OBJECT			m_COO;
	bool			m_bCOO;
	uint64_t		m_timeOutVerify;
	uint64_t		m_timeOutShow;
	uint64_t		m_tIntSend;
	uint64_t		m_tLastSentCOO;

	float m_cSpeed; //conveyer speed
	float m_cLen;	//conveyer length
	float m_minOverlap;

};

}
#endif
