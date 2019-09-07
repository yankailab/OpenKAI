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
	bool draw(void);
	bool console(int& iY);

	void handleCMD(void);

	void handleBtn(int iBtn, int state);
	static void callbackCMD(int iBtn, int state, void* pInst)
	{
		NULL_(pInst);
		_SortingImgClient* pC = (_SortingImgClient*)pInst;
		pC->handleBtn(iBtn, state);
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
	vFloat4 m_bbShow;

};

}
#endif
