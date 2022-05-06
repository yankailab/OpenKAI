/*
 * Window.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#include "_WindowCV.h"
#include "../Script/Kiss.h"

namespace kai
{

	_WindowCV::_WindowCV()
	{
		m_waitKey = 30;
		m_bFullScreen = false;
		m_gstOutput = "";
		m_vSize.set(1280, 720);
		m_bShow = true;
	}

	_WindowCV::~_WindowCV()
	{
	}

	bool _WindowCV::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		vector<string> vB;
		pK->a("vBASE", &vB);
		for (string p : vB)
		{
			BASE *pB = (BASE *)(pK->getInst(p));
			IF_CONT(!pB);

			m_vpB.push_back(pB);
		}

		pK->v("bShow", &m_bShow);
		pK->v("bFullScreen", &m_bFullScreen);
		pK->v("vSize", &m_vSize);
		m_waitKey = 1000.0f / m_pT->getTargetFPS();

		if (m_vSize.area() <= 0)
		{
			LOG_E("Window size too small");
			return false;
		}

		pK->v("gstOutput", &m_gstOutput);
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

		m_sF.get()->allocate(m_vSize.x, m_vSize.y);
		m_sF.next()->allocate(m_vSize.x, m_vSize.y);

		IF_T(!m_bShow);

		string wn = *this->getName();

		if (m_bFullScreen)
		{
			namedWindow(wn, WINDOW_NORMAL);
			setWindowProperty(wn, WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
		}
		else
		{
			namedWindow(wn, WINDOW_AUTOSIZE);
		}

		setMouseCallback(wn, sOnMouse, this);

		// UI
		// Freetype font
		pK->v("font", &m_font);
		if (m_font != "")
		{
			m_pFT = freetype::createFreeType2();
			m_pFT->loadFontData(m_font.c_str(), 0);
		}

		// buttons
		Kiss *pKwb = pK->child("btn");
		IF_T(pKwb->empty());
		WindowCV_Btn wb;
		int i = 0;
		while (1)
		{
			Kiss *pKb = pKwb->child(i++);
			if (pKb->empty())
				break;

			pKb->v("name", &wb.m_name);
			pKb->v("label", &wb.m_label);
			pKb->v("bShow", &wb.m_bShow);
			pKb->v("vRegion", &wb.m_vRegion);
			pKb->v("colUp", &wb.m_colUp);
			pKb->v("colDown", &wb.m_colDown);
			pKb->v("colBorder", &wb.m_colBorder);
			pKb->v("colFont", &wb.m_colFont);
			pKb->v("hFont", &wb.m_hFont);
			wb.init(m_sF.get()->size(), m_pFT);

			m_vBtn.push_back(wb);
		}

		return true;
	}

	bool _WindowCV::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _WindowCV::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateWindow();

			m_pT->autoFPSto();
		}
	}

	void _WindowCV::updateWindow(void)
	{
		// draw contents
		for (BASE *pB : m_vpB)
		{
			pB->cvDraw(this);
		}

		IF_(m_sF.next()->bEmpty());

		// draw UI
		for (WindowCV_Btn wb : m_vBtn)
		{
			wb.draw(m_sF.next()->m(), m_pFT);
		}

		m_sF.swap();
		*m_sF.next()->m() = Scalar(0, 0, 0);

		// show window
		if (m_bShow)
		{
			imshow(*this->getName(), *m_sF.get()->m());
		}

		Frame F,F2;
		if (m_gst.isOpened())
		{
			F.copy(*m_sF.get());
			Size fs = F.size();

			if (fs.width != m_vSize.x || fs.height != m_vSize.y)
			{
				F2 = F.resize(m_vSize.x, m_vSize.y);
				F = F2;
			}

			// cuda convert crash on Jetson?
			// if (F.m()->type() != CV_8UC3)
			// {
			// 	F2 = F.cvtColor(COLOR_GRAY2BGR);
			// 	F = F2;
			// }

			m_gst << *F.m();
		}

		int key = waitKey(m_waitKey);
	}

	Frame *_WindowCV::getFrame(void)
	{
		return m_sF.get();
	}

	Frame *_WindowCV::getNextFrame(void)
	{
		return m_sF.next();
	}

	cv::Ptr<freetype::FreeType2> _WindowCV::getFont(void)
	{
		return m_pFT;
	}

	WindowCV_Btn* _WindowCV::findBtn(const string& btnName)
	{
		for (int i = 0; i < m_vBtn.size(); i++)
		{
			WindowCV_Btn *pB = &m_vBtn[i];
			IF_CONT(btnName != pB->m_name);
			return pB;
		}

		return NULL;
	}

	bool _WindowCV::setBtnLabel(const string &btnName, const string &btnLabel)
	{
		WindowCV_Btn* pB = findBtn(btnName);
		NULL_F(pB);

		pB->setLabel(btnLabel, m_pFT);
		return true;
	}

	bool _WindowCV::setBtnVisible(const string &btnName, bool bShow)
	{
		WindowCV_Btn* pB = findBtn(btnName);
		NULL_F(pB);

		pB->setVisible(bShow);
		return true;
	}

	void _WindowCV::setBtnVisibleAll(bool bShow)
	{
		for (int i = 0; i < m_vBtn.size(); i++)
		{
			WindowCV_Btn *pB = &m_vBtn[i];
			pB->setVisible(bShow);
		}
	}

	bool _WindowCV::setCbBtn(const string &btnName, CbWindowCVbtn pCb, void *pInst)
	{
		NULL_F(pInst);

		WindowCV_Btn* pB = findBtn(btnName);
		NULL_F(pB);

		pB->setCallback(pCb, pInst);
		return true;
	}

	void _WindowCV::sOnMouse(int event, int x, int y, int flags, void *userdata)
	{
		NULL_(userdata);

		_WindowCV *pW = (_WindowCV *)userdata;
		pW->onMouse(event, x, y, flags);
	}

	void _WindowCV::onMouse(int event, int x, int y, int flags)
	{
		for (int i = 0; i < m_vBtn.size(); i++)
		{
			WindowCV_Btn *pB = &m_vBtn[i];

			if (event == EVENT_LBUTTONDOWN)
				pB->mouseDown(x, y);
			else if (event == EVENT_MOUSEMOVE)
				pB->mouseMove(x, y);
			else if (event == EVENT_LBUTTONUP)
				pB->mouseUp(x, y);
		}
	}
}
