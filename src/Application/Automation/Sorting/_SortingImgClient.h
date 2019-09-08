#ifndef OpenKAI_src_Automation__SortingImgClient_H_
#define OpenKAI_src_Automation__SortingImgClient_H_

#include "../../../Protocol/_OKlink.h"
#include "../../../Detector/_DetectorBase.h"

namespace kai
{

class _SortingImgClient: public _OKlink
{
public:
	_SortingImgClient();
	~_SortingImgClient();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	bool draw(void);
	bool console(int& iY);

	void handleCMD(void);

	void handleBtn(int id, int state);
	static void callbackBtn(int id, int state, void* pInst)
	{
		NULL_(pInst);
		_SortingImgClient* pC = (_SortingImgClient*)pInst;
		pC->handleBtn(id, state);
	}

	void handleMouse(int event, float x, float y);
	static void callbackMouse(int event, float x, float y, void* pInst)
	{
		NULL_(pInst);
		_SortingImgClient* pC = (_SortingImgClient*)pInst;
		pC->handleMouse(event, x, y);
	}

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SortingImgClient *) This)->update();
		return NULL;
	}

public:
	OBJECT	m_COO;
	vFloat4 m_bbWin;

	bool m_bDrag;
	vFloat2 m_vDragFrom;
	vFloat2 m_vDragTo;

};

}
#endif
