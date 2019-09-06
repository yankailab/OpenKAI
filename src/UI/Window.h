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

#define TAB_PIX 20
#define LINE_HEIGHT 20
#define MOUSE_L 1
#define MOUSE_M (1<<1)
#define MOUSE_R (1<<2)
#define BTN_DOWN 0
#define BTN_UP 1

typedef void (*CallbackBtn)(int state);

struct WINDOW_BUTTON
{
	vFloat4 m_bb;
	Mat m_mUp;
	Mat m_mDown;
	Mat m_mUpScale;
	Mat m_mDownScale;
	vInt2 m_mSize;
	bool m_bDown;

	CallbackBtn m_cbBtn;

	void init(void)
	{
		m_bb.init();
		m_mSize.init();
		m_bDown = false;
		m_cbBtn = NULL;
	}

	void onMouse(int event, vFloat2& p)
	{
		switch (event)
		{
		case EVENT_MOUSEMOVE:
			if(!bMouseOn(p))
			{
				m_bDown = false;
				if(m_cbBtn)
					m_cbBtn(BTN_UP);
			}
			break;
		case EVENT_LBUTTONDOWN:
			if(bMouseOn(p))
			{
				m_bDown = true;
				if(m_cbBtn)
					m_cbBtn(BTN_DOWN);
			}
			break;
		case EVENT_LBUTTONUP:
			m_bDown = false;
			if(m_cbBtn)
				m_cbBtn(BTN_UP);
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

	void setBtnCallback(CallbackBtn cb)
	{
		m_cbBtn = cb;
	}

	void draw(Mat* pM)
	{
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

		if(m_bDown)
			m_mUpScale.copyTo((*pM)(r));
		else
			m_mDownScale.copyTo((*pM)(r));
	}

};

class Window: public BASE
{
public:
	Window();
	virtual ~Window();

	bool init(void *pKiss);
	bool draw(void);
	Frame* getFrame(void);
	Point* getTextPos(void);

	void addMsg(const string &pMsg);
	void tabNext(void);
	void tabPrev(void);
	void tabReset(void);
	void lineNext(void);
	void lineReset(void);
	double textSize(void);
	Scalar textColor(void);

	bool bMouseButton(uint32_t fB);
	void OnMouse(int event, int x, int y);

	static void callBackMouse(int event, int x, int y, int flags, void *pW)
	{
		((Window*) pW)->OnMouse(event, x, y);
	}

public:
	bool m_bWindow;
	bool m_bFullScreen;
	Frame m_frame;
	vInt2 m_size;
	vInt2 m_textPos;
	vInt2 m_textStart;
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

	vector<WINDOW_BUTTON> m_vBtn;

};

}

#endif
