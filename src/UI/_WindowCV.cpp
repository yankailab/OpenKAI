/*
 * Window.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#include "_WindowCV.h"

namespace kai
{

	_WindowCV::_WindowCV()
	{
		m_waitKey = 30;
		m_bFullScreen = false;
		m_vSize.set(1280, 720);
	}

	_WindowCV::~_WindowCV()
	{
	}

	bool _WindowCV::init(void *pKiss)
	{
		IF_F(!this->_UIbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bFullScreen", &m_bFullScreen);
		pK->v("vSize", &m_vSize);
		m_waitKey = 1000.0f / m_pT->getTargetFPS();

		if (m_vSize.area() <= 0)
		{
			LOG_E("Window size too small");
			return false;
		}

		m_F.allocate(m_vSize.x, m_vSize.y);

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
#ifdef USE_FREETYPE
		// Freetype font
		pK->v("font", &m_font);
		if (m_font != "")
		{
			m_pFT = freetype::createFreeType2();
			m_pFT->loadFontData(m_font.c_str(), 0);
		}
#endif

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
#ifdef USE_FREETYPE
			wb.init(m_F.size(), &m_pFT);
#else
			wb.init(m_F.size());
#endif
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
		while (m_pT->bThread())
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
			pB->draw((void *)&m_F);
		}
		IF_(m_F.bEmpty());

		// draw UI
		for (WindowCV_Btn wb : m_vBtn)
		{
#ifdef USE_FREETYPE
			wb.draw(m_F.m(), &m_pFT);
#else
			wb.draw(m_F.m());
#endif
		}

		// show window
		imshow(*this->getName(), *m_F.m());

		int key = waitKey(m_waitKey);
	}

#ifdef USE_FREETYPE
	cv::Ptr<freetype::FreeType2> _WindowCV::getFont(void)
	{
		return m_pFT;
	}
#endif

	WindowCV_Btn *_WindowCV::findBtn(const string &btnName)
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
		WindowCV_Btn *pB = findBtn(btnName);
		NULL_F(pB);

#ifdef USE_FREETYPE
		pB->setLabel(btnLabel, &m_pFT);
#else
		pB->setLabel(btnLabel);
#endif
		return true;
	}

	bool _WindowCV::setBtnVisible(const string &btnName, bool bShow)
	{
		WindowCV_Btn *pB = findBtn(btnName);
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

		WindowCV_Btn *pB = findBtn(btnName);
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
