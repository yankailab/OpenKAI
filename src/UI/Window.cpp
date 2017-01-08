/*
 * Window.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#include "Window.h"
#include "../Script/Kiss.h"

namespace kai
{

Window::Window()
{
	m_pFrame = NULL;
	m_textPos.init();
	m_textStart.init();
	m_size.init();
	m_bFullScreen = false;
	m_pixTab = TAB_PIX;
	m_lineHeight = LINE_HEIGHT;
	m_textSize = 0.6;
	m_textCol = Scalar(0,255,0);
	m_bShow = true;
}

Window::~Window()
{
	if(m_VW.isOpened())
	{
		m_VW.release();
	}
}

bool Window::init(void* pKiss)
{
	CHECK_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_ERROR_F(pK->v("w", &m_size.m_x));
	F_ERROR_F(pK->v("h", &m_size.m_y));
	F_INFO(pK->v("bFullScreen", &m_bFullScreen));

	if(m_size.area()<=0)
	{
		LOG_E("Window size too small");
		return false;
	}

	F_INFO(pK->v("bShow", &m_bShow));
	F_INFO(pK->v("textX", &m_textStart.m_x));
	F_INFO(pK->v("textY", &m_textStart.m_y));
	F_INFO(pK->v("pixTab", &m_pixTab));
	F_INFO(pK->v("lineH", &m_lineHeight));
	F_INFO(pK->v("textSize", &m_textSize));
	F_INFO(pK->v("textB", &m_textCol[0]));
	F_INFO(pK->v("textG", &m_textCol[1]));
	F_INFO(pK->v("textR", &m_textCol[2]));

	string fileRec = "";
	F_INFO(pK->v("fileRec", &fileRec));
	if(!fileRec.empty())
	{
		int recordFPS = 30;
		string recordCodec = "MJPG";
		F_INFO(pK->v("fpsRec", &recordFPS));
		F_INFO(pK->v("codecRec", &recordCodec));

		time_t t = time(NULL);
	    struct tm *tm = localtime(&t);
	    char strTime[128];
	    strftime(strTime, sizeof(strTime), "%c", tm);
	    fileRec += strTime;
	    fileRec += ".avi";

		CHECK_F(!m_VW.open(
				fileRec,
				CV_FOURCC(
						recordCodec.at(0),
						recordCodec.at(1),
						recordCodec.at(2),
						recordCodec.at(3)),
				recordFPS,
				cv::Size(m_size.m_x,
						m_size.m_y)
				));
	}

	m_pFrame = new Frame();
	m_pFrame->allocate(m_size.m_x, m_size.m_y);

	if (m_bFullScreen)
	{
		namedWindow(*this->getName(), CV_WINDOW_NORMAL);
		setWindowProperty(*this->getName(),
				CV_WND_PROP_FULLSCREEN,
				CV_WINDOW_FULLSCREEN);
	}
	else
	{
		namedWindow(*this->getName(), CV_WINDOW_AUTOSIZE);
	}
//	setMouseCallback(*this->getName(), onMouseGeneral, NULL);

	return true;
}

bool Window::link(void)
{
	return true;
}

bool Window::draw(void)
{
	NULL_F(m_pFrame);
	CHECK_F(m_pFrame->empty());

	if(m_bShow)
	{
		imshow(*this->getName(), *m_pFrame->getCMat());
	}

	if(m_VW.isOpened())
	{
		m_VW << *m_pFrame->getCMat();
	}

	m_pFrame->allocate(m_size.m_x, m_size.m_y);
	tabReset();
	lineReset();

	return true;
}

Frame* Window::getFrame(void)
{
	return m_pFrame;
}

Point* Window::getTextPos(void)
{
	m_tPoint.x = m_textPos.m_x;
	m_tPoint.y = m_textPos.m_y;
	return &m_tPoint;
}

void Window::tabNext(void)
{
	m_textPos.m_x += m_pixTab;
}

void Window::tabPrev(void)
{
	m_textPos.m_x -= m_pixTab;
	if(m_textPos.m_x < m_textStart.m_x)m_textPos.m_x = m_textStart.m_x;
}

void Window::tabReset(void)
{
	m_textPos.m_x = m_textStart.m_x;
}

void Window::lineNext(void)
{
	m_textPos.m_y += m_lineHeight;
}

void Window::lineReset(void)
{
	m_textPos.m_y = m_textStart.m_y;
}

double Window::textSize(void)
{
	return m_textSize;
}

Scalar Window::textColor(void)
{
	return m_textCol;
}

void Window::addMsg(string* pMsg)
{
	putText(*m_pFrame->getCMat(),
			*pMsg,
			*getTextPos(),
			FONT_HERSHEY_SIMPLEX,
			m_textSize,
			m_textCol,
			1);
	lineNext();
}



}
