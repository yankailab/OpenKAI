#ifndef OpenKAI_src_Automation__SortingImgClient_H_
#define OpenKAI_src_Automation__SortingImgClient_H_

#include "../../../Protocol/_OKlink.h"
#include "../../../Detector/_DetectorBase.h"
#include "_SortingCtrlServer.h"

namespace kai
{

class _SortingCtrlClient: public _OKlink
{
public:
	_SortingCtrlClient();
	~_SortingCtrlClient();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	bool draw(void);
	bool console(int& iY);

	void handleCMD(void);
	void updateWindow(void);

	void onBtn(int id, int state);
	static void callbackBtn(int id, int state, void* pInst)
	{
		NULL_(pInst);
		_SortingCtrlClient* pC = (_SortingCtrlClient*)pInst;
		pC->onBtn(id, state);
	}

	void onMouse(int event, float x, float y);
	static void callbackMouse(int event, float x, float y, void* pInst)
	{
		NULL_(pInst);
		_SortingCtrlClient* pC = (_SortingCtrlClient*)pInst;
		pC->onMouse(event, x, y);
	}

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SortingCtrlClient *) This)->update();
		return NULL;
	}

public:
	OBJECT	m_COO;
	uint64_t m_tLastSent;
	uint64_t m_tSendInt;

	int	m_iState;
	int	m_iSetState;

	vFloat2 m_vBB;
	bool	m_bMouse;
	vFloat2 m_vMouse;

};

}
#endif
