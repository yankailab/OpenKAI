/*
 * Window.h
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_WindowCV_H_
#define OpenKAI_src_UI_WindowCV_H_
#ifdef USE_OPENCV
#include "../Base/_ModuleBase.h"
#include "../Vision/Frame.h"

namespace kai
{

typedef void (*CbWindowCVMouse)(int event, int x, int y, int flags, void* pInst);
struct WindowCV_CbMouse
{
	CbWindowCVMouse m_pCb = NULL;
	void *m_pInst = NULL;

	void add(CbWindowCVMouse pCb, void *pPinst)
	{
		m_pCb = pCb;
		m_pInst = pPinst;
	}

	bool bValid(void)
	{
		return (m_pCb && m_pInst) ? true : false;
	}

	void call(int event, int x, int y, int flags)
	{
		if (!bValid())
			return;

		m_pCb(event, x, y, flags, m_pInst);
	}
};

class _WindowCV: public _ModuleBase
{
public:
	_WindowCV();
	virtual ~_WindowCV();

	bool init(void *pKiss);
	bool start(void);

	Frame* getFrame(void);

	void setCbMouse(CbWindowCVMouse pCb, void *pInst);

protected:
	void updateWindow(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_WindowCV*) This)->update();
		return NULL;
	}

	static void OnMouse(int event, int x, int y, int flags, void* userdata);

public:
	vector<BASE*> m_vpB;
	int	m_waitKey;

	bool m_bShow;
	bool m_bFullScreen;

	Frame m_frame;
	vInt2 m_vSize;

	string m_gstOutput;
	VideoWriter m_gst;

	string m_fileRec;
	VideoWriter m_VW;

	Frame m_F;
	Frame m_F2;

	// callback
	WindowCV_CbMouse m_cbMouse;
};

}
#endif
#endif
