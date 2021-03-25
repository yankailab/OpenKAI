/*
 * Window.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#ifdef USE_OPENCV
#include "CVwindow.h"
#include "../Script/Kiss.h"

namespace kai
{

CVwindow::CVwindow()
{
	m_textY = 0;
	m_vTextStart.init(20,20);
	m_vSize.init(1280,720);
	m_bFullScreen = false;
	m_pixTab = 20;
	m_lineHeight = 20;
	m_textSize = 0.5;
	m_textCol = Scalar(0, 255, 0);
	m_bWindow = true;
	m_bDrawMsg = true;
	m_gstOutput = "";
	m_fileRec = "";
}

CVwindow::~CVwindow()
{
	if (m_VW.isOpened())
	{
		m_VW.release();
	}
}

bool CVwindow::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->root()->child("APP")->v("bWindow", &m_bWindow);
	if (!m_bWindow)
		LOG_I("Window mode is disabled. Turn \"bWindow\":1 to enable");

	pK->v("bFullScreen",&m_bFullScreen);
	pK->v("bDrawMsg", &m_bDrawMsg);
	pK->v("vSize", &m_vSize);

	if (m_vSize.area() <= 0)
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
						cv::Size(m_vSize.x, m_vSize.y)))
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
						cv::Size(m_vSize.x, m_vSize.y),
						true))
		{
			LOG_E("Cannot open GStreamer output");
			return false;
		}
	}

	pK->v("vTextStart", &m_vTextStart);
	pK->v("pixTab", &m_pixTab);
	pK->v("lineH", &m_lineHeight);
	pK->v("textSize", &m_textSize);
	pK->v("textCol", &m_textCol);

	m_frame.allocate(m_vSize.x, m_vSize.y);

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

	return true;
}

void CVwindow::draw(void)
{
	IF_(m_frame.bEmpty());

	Mat m = *m_frame.m();

	if (m_bWindow)
	{
		imshow(*this->getName(), m);
	}

	if(m_VW.isOpened() || m_gst.isOpened())
	{
		m_F.copy(m_frame);
		Size fs = m_F.size();

		if (fs.width != m_vSize.x || fs.height != m_vSize.y)
		{
			m_F2 = m_F.resize(m_vSize.x, m_vSize.y);
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

	m_textY = m_vTextStart.y;
	*m_frame.m() = Scalar(0,0,0);
}

Frame* CVwindow::getFrame(void)
{
	return &m_frame;
}

double CVwindow::textSize(void)
{
	return m_textSize;
}

Scalar CVwindow::textColor(void)
{
	return m_textCol;
}

void CVwindow::addMsg(const string& pMsg, int iTab)
{
	IF_(!m_bDrawMsg);

	Point p;
	p.x = m_vTextStart.x + iTab * m_pixTab;
	p.y = m_textY;

	putText(*m_frame.m(), pMsg, p, FONT_HERSHEY_SIMPLEX,
			m_textSize, m_textCol, 1);
	m_textY += m_lineHeight;
}

}
#endif
