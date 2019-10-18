/*
 * Window.h
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_Windows_H_
#define OpenKAI_src_UI_Windows_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "../Vision/Frame.h"

namespace kai
{

#define MOUSE_MOVE 0
#define MOUSE_L 1
#define MOUSE_M (1<<1)
#define MOUSE_R (1<<2)
#define BTN_DOWN 0
#define BTN_UP 1

typedef void (*CallbackBtn)(int iBtn, int state, void* pfInst);
typedef void (*CallbackMouse)(int event, float x, float y, void* pfInst);

struct WINDOW_BUTTON
{
	bool m_bEnable;
	int	m_id;
	vFloat4 m_bb;
	Mat m_mUp;
	Mat m_mDown;
	Mat m_mUpScale;
	Mat m_mDownScale;
	vInt2 m_mSize;
	bool m_bDown;
	bool m_bShowDown;

	CallbackBtn m_pfBtn;
	void*		m_pfInst;

	void init(void)
	{
		m_bEnable = true;
		m_id = -1;
		m_bb.init();
		m_mSize.init();
		m_bDown = false;
		m_pfBtn = NULL;
		m_pfInst = NULL;
		m_bShowDown = false;
	}

	void setEnable(bool bEnable)
	{
		m_bEnable = bEnable;
	}

	void setShownDown(bool bShowDown)
	{
		m_bShowDown = bShowDown;
	}

	void onMouse(int event, vFloat2& p)
	{
		IF_(!m_bEnable);

		switch (event)
		{
		case EVENT_MOUSEMOVE:
			if(!bMouseOn(p))
			{
				m_bDown = false;
				if(m_pfBtn)
					m_pfBtn(m_id, BTN_UP, m_pfInst);
			}
			break;
		case EVENT_LBUTTONDOWN:
			if(bMouseOn(p))
			{
				m_bDown = true;
				if(m_pfBtn)
					m_pfBtn(m_id, BTN_DOWN, m_pfInst);
			}
			break;
		case EVENT_LBUTTONUP:
			m_bDown = false;
			if(m_pfBtn)
				m_pfBtn(m_id, BTN_UP, m_pfInst);
			break;
		}
	}

	bool bMouseOn(vFloat2& p)
	{
		IF_F(p.x < m_bb.x);
		IF_F(p.x > m_bb.z);
		IF_F(p.y < m_bb.y);
		IF_F(p.y > m_bb.w);

		return true;
	}

	void drawBtn(Mat* pM)
	{
		IF_(!m_bEnable);
		NULL_(pM);
		IF_(pM->cols <= 0);
		IF_(pM->rows <= 0);

		if(m_mSize.x != pM->cols || m_mSize.y != pM->rows)
		{
			m_mSize.x = pM->cols;
			m_mSize.y = pM->rows;

			int w = m_bb.width() * pM->cols;
			int h = m_bb.height() * pM->rows;

			cv::resize(m_mUp,m_mUpScale,Size(w,h));
			cv::resize(m_mDown,m_mDownScale,Size(w,h));
		}

		Rect r = Rect(m_bb.x*pM->cols, m_bb.y*pM->rows, m_mUpScale.cols, m_mUpScale.rows);

		if(m_bShowDown)
			m_mDownScale.copyTo((*pM)(r));
		else if(m_bDown)
			m_mDownScale.copyTo((*pM)(r));
		else
			m_mUpScale.copyTo((*pM)(r));
	}

	void setBtnCallback(CallbackBtn cb, void* pfInst)
	{
		m_pfBtn = cb;
		m_pfInst = pfInst;
	}

};

class Window: public BASE
{
public:
	Window();
	virtual ~Window();

	bool init(void *pKiss);
	void draw(void);
	Frame* getFrame(void);

	void addMsg(const string &pMsg, int iTab=0);
	double textSize(void);
	Scalar textColor(void);

	WINDOW_BUTTON* getBtn(int id);
	void resetAllBtn(void);

	bool bMouseButton(uint32_t fB);
	void OnMouse(int event, int x, int y);
	void addCallbackMouse(CallbackMouse cb, void* pfInst);

	static void callBackMouse(int event, int x, int y, int flags, void *pW)
	{
		Window* pWin = (Window*)pW;
		pWin->OnMouse(event, x, y);
	}

public:
	bool m_bWindow;
	bool m_bFullScreen;
	bool m_bDrawMsg;
	Frame m_frame;
	vInt2 m_size;
	vInt2 m_textStart;
	int m_textY;
	int m_pixTab;
	int m_lineHeight;
	Point m_tPoint;
	double m_textSize;
	Scalar m_textCol;

	string m_gstOutput;
	VideoWriter m_gst;

	string m_fileRec;
	VideoWriter m_VW;

	Frame m_F;
	Frame m_F2;

	//mouse handler
	bool m_bMouse;
	vFloat2 m_vMouse;
	uint32_t m_fMouse;
	bool m_bShowMouse;

	CallbackMouse m_pfMouse;
	void*		m_pfInst;

	vector<WINDOW_BUTTON> m_vBtn;

};

}

#endif
