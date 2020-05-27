/*
 * Window.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#include "Window.h"

#ifdef USE_OPENCV
#include "../Script/Kiss.h"

namespace kai
{

Window::Window()
{
	m_textY = 0;
	m_textStart.x = 20;
	m_textStart.y = 20;
	m_size.x = 1280;
	m_size.y = 720;
	m_bFullScreen = false;
	m_pixTab = 20;
	m_lineHeight = 20;
	m_textSize = 0.5;
	m_textCol = Scalar(0, 255, 0);
	m_bWindow = true;
	m_bDrawMsg = true;
	m_bMouse = false;
	m_fMouse = 0;
	m_bShowMouse = false;
	m_gstOutput = "";
	m_fileRec = "";

	m_pfMouse = NULL;
	m_pfInst = NULL;
}

Window::~Window()
{
	if (m_VW.isOpened())
	{
		m_VW.release();
	}
}

bool Window::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->root()->o("APP")->v("bWindow", &m_bWindow);
	if (!m_bWindow)
		LOG_I("Window mode is disabled. Turn \"bWindow\":1 to enable");

	pK->v("bFullScreen",&m_bFullScreen);
	pK->v("bDrawMsg", &m_bDrawMsg);
	pK->v("w", &m_size.x);
	pK->v("h", &m_size.y);

	if (m_size.area() <= 0)
	{
		LOG_E("Window size too small");
		return false;
	}

	pK->v("fileRec",&m_fileRec);
	if (!m_fileRec.empty())
	{
		int recFPS = 30;
		string reCodec = "MJPG";
		pK->v("recFPS", &recFPS);
		pK->v("recCodec", &reCodec);

		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		char strTime[128];
		strftime(strTime, sizeof(strTime), "%c", tm);
		m_fileRec += strTime;
		m_fileRec += ".avi";

		if (!m_VW.open(m_fileRec,
#if CV_VERSION_MAJOR==3
				CV_FOURCC
#else
				VideoWriter::fourcc
#endif
						(reCodec.at(0),
						reCodec.at(1),
						reCodec.at(2),
						reCodec.at(3)),
						recFPS,
						cv::Size(m_size.x, m_size.y)))
		{
			LOG_E("Cannot open file recording");
			return false;
		}
	}

	pK->v("gstOutput",&m_gstOutput);
	if (!m_gstOutput.empty())
	{
		if (!m_gst.open(m_gstOutput,
						CAP_GSTREAMER,
						0,
						30,
						cv::Size(m_size.x, m_size.y),
						true))
		{
			LOG_E("Cannot open GStreamer output");
			return false;
		}
	}

	pK->v("textStart", &m_textStart);
	pK->v("pixTab", &m_pixTab);
	pK->v("lineH", &m_lineHeight);
	pK->v("textSize", &m_textSize);
	pK->v("textCol", &m_textCol);

	m_frame.allocate(m_size.x, m_size.y);

	IF_T(!m_bWindow);

	if (m_bFullScreen)
	{
		namedWindow(*this->getName(), WINDOW_NORMAL);
		setWindowProperty(*this->getName(), WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
	}
	else
	{
		namedWindow(*this->getName(), WINDOW_AUTOSIZE);
	}

	pK->v("bMouse",&m_bMouse);
	pK->v("bShowMouse",&m_bShowMouse);
	if(m_bMouse)
	{
		setMouseCallback(*this->getName(), callBackMouse, this);
	}

	Kiss* pB = pK->o("button");
	if(pB)
	{
		Kiss** ppB = pB->getChildItr();

		int i=0;
		while((pB=ppB[i++]))
		{
			WINDOW_BUTTON wb;
			wb.init();
			pB->v("id",&wb.m_id);
			pB->v("bb",&wb.m_bb);

			string fBtn;
			pB->v("fBtnUp",&fBtn);
			wb.m_mUp = cv::imread(fBtn);
			pB->v("fBtnDown",&fBtn);
			wb.m_mDown = cv::imread(fBtn);

			m_vBtn.push_back(wb);
		}
	}

	return true;
}

void Window::draw(void)
{
	IF_(m_frame.bEmpty());

	Mat m = *m_frame.m();

	for(int i=0; i<m_vBtn.size(); i++)
	{
		WINDOW_BUTTON* pB = &m_vBtn[i];
		pB->drawBtn(m_frame.m());
	}

	if(m_bShowMouse)
	{
		if(bMouseButton(MOUSE_L))
			circle(m, Point(m_vMouse.x * m.cols, m_vMouse.y * m.rows), 15, m_textCol, 10);
	}

	if (m_bWindow)
	{
		imshow(*this->getName(), m);
	}

	if(m_VW.isOpened() || m_gst.isOpened())
	{
		m_F.copy(m_frame);
		Size fs = m_F.size();

		if (fs.width != m_size.x || fs.height != m_size.y)
		{
			m_F2 = m_F.resize(m_size.x, m_size.y);
			m_F = m_F2;
		}

		if (m_F.m()->type() != CV_8UC3)
		{
			m_F2 = m_F.cvtColor(COLOR_GRAY2BGR);
			m_F = m_F2;
		}

		if (m_VW.isOpened())
			m_VW << *m_F.m();

		if (m_gst.isOpened())
			m_gst << *m_F.m();
	}

	m_textY = m_textStart.y;
	*m_frame.m() = Scalar(0,0,0);
}

Frame* Window::getFrame(void)
{
	return &m_frame;
}

double Window::textSize(void)
{
	return m_textSize;
}

Scalar Window::textColor(void)
{
	return m_textCol;
}

void Window::addMsg(const string& pMsg, int iTab)
{
	IF_(!m_bDrawMsg);

	Point p;
	p.x = m_textStart.x + iTab * m_pixTab;
	p.y = m_textY;

	putText(*m_frame.m(), pMsg, p, FONT_HERSHEY_SIMPLEX,
			m_textSize, m_textCol, 1);
	m_textY += m_lineHeight;
}

WINDOW_BUTTON* Window::getBtn(int id)
{
	for(int i=0; i<m_vBtn.size(); i++)
	{
		WINDOW_BUTTON* pB = &m_vBtn[i];
		IF_CONT(pB->m_id != id);

		return pB;
	}

	return NULL;
}

void Window::resetAllBtn(void)
{
	for(int i=0; i<m_vBtn.size(); i++)
	{
		WINDOW_BUTTON* pB = &m_vBtn[i];
		pB->setShownDown(false);
	}
}

bool Window::bMouseButton(uint32_t fB)
{
	return m_fMouse & fB;
}

void Window::OnMouse(int event, int x, int y)
{
	Frame* pF = this->getFrame();
	NULL_(pF);

	m_vMouse.x = (float)x/(float)pF->m()->cols;
	m_vMouse.y = (float)y/(float)pF->m()->rows;

	switch (event)
	{
	case EVENT_MOUSEMOVE:
		break;
	case EVENT_LBUTTONDOWN:
		m_fMouse |= MOUSE_L;
		break;
	case EVENT_LBUTTONUP:
		m_fMouse &= ~MOUSE_L;
		break;
	case EVENT_RBUTTONDOWN:
		m_fMouse |= MOUSE_R;
		break;
	case EVENT_RBUTTONUP:
		m_fMouse &= ~MOUSE_R;
		break;
	case EVENT_MBUTTONDOWN:
		m_fMouse |= MOUSE_M;
		break;
	case EVENT_MBUTTONUP:
		m_fMouse &= ~MOUSE_M;
		break;
	default:
		break;
	}

	for(int i=0; i<m_vBtn.size(); i++)
	{
		WINDOW_BUTTON* pB = &m_vBtn[i];
		pB->onMouse(event, m_vMouse);
	}

	if(m_pfMouse)
		m_pfMouse(event, m_vMouse.x, m_vMouse.y, m_pfInst);

}

void Window::addCallbackMouse(CallbackMouse cb, void* pfInst)
{
	m_pfMouse = cb;
	m_pfInst = pfInst;
}

}
#endif
