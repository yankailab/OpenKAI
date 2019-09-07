/*
 * _SortingImgServer.h
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Automation__SortingImgServer_H_
#define OpenKAI_src_Automation__SortingImgServer_H_

#include "../../../Detector/_DetectorBase.h"
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

namespace kai
{

class _SortingImgServer: public _DetectorBase
{
public:
	_SortingImgServer(void);
	virtual ~_SortingImgServer();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

	void handleCMD(uint8_t* pCMD);
	static void callbackCMD(uint8_t* pCMD, void* pInst)
	{
		NULL_(pInst);
		_SortingImgServer* pS = (_SortingImgServer*)pInst;
		pS->handleCMD(pCMD);
	}

private:
	void updateImg(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SortingImgServer *) This)->update();
		return NULL;
	}

public:
	_OKlink*		m_pOL;

	int				m_ID;
	OBJECT			m_COO;
	bool			m_bCOO;
	uint64_t		m_timeOutVerify;
	uint64_t		m_timeOutShow;
	uint64_t		m_tIntSend;
	uint64_t		m_tLastSent;

	float m_cSpeed; //conveyer speed
	float m_cLen;	//conveyer length
	float m_minOverlap;

};

}
#endif
