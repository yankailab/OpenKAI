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
}

Window::~Window()
{
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

	F_INFO(pK->v("textX", &m_textStart.m_x));
	F_INFO(pK->v("textY", &m_textStart.m_y));
	F_INFO(pK->v("pixTab", &m_pixTab));
	F_INFO(pK->v("lineH", &m_lineHeight));

	m_pFrame = new Frame();

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

	m_pFrame->allocate(m_size.m_x, m_size.m_y);
	imshow(*this->getName(), *m_pFrame->getCMat());

	return true;
}

Frame* Window::getFrame(void)
{
	return m_pFrame;
}

Point* Window::getTextPos(void)
{
	static Point p(m_textPos.m_x, m_textPos.m_y);
	return &p;
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

} /* namespace kai */
