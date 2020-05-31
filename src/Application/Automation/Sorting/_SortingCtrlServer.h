/*
 * _SortingImgServer.h
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Automation__SortingImgServer_H_
#define OpenKAI_src_Automation__SortingImgServer_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Protocol/_ProtocolBase.h"
#include "../../../Vision/_DepthVisionBase.h"

#ifdef USE_OPENCV

//0 PROTOCOL_BEGIN
//1 COMMAND
//2 PAYLOAD LENGTH
//3 X
//4 Y
//5 iClass

#define SORTINGCTRL_STATE 0
#define SORTINGCTRL_OBJ 5
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
	void draw(void);
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
	_ProtocolBase*	m_pPB;
	_Object			m_newO;
	float			m_bbSize;
	float			m_dT;
	int				m_iState;
	INTERVAL_EVENT	m_ieState;

	float m_cSpeed; //conveyer speed
	float m_cLen;	//conveyer length

};

}
#endif
#endif
