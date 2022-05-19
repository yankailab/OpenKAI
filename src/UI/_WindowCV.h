/*
 * Window.h
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_WindowCV_H_
#define OpenKAI_src_UI_WindowCV_H_

#ifdef USE_FREETYPE
#include <opencv2/freetype.hpp>
#endif

#include <opencv2/highgui.hpp>
#include "../Base/_ModuleBase.h"
#include "../Vision/Frame.h"
#include "../Primitive/tSwap.h"
#include "../Utility/utilCV.h"

namespace kai
{
	typedef void (*CbWindowCVbtn)(void *pInst, uint32_t flag);
	struct WindowCV_CbBtn
	{
		CbWindowCVbtn m_pCb = NULL;
		void *m_pInst = NULL;

		void add(CbWindowCVbtn pCb, void *pPinst)
		{
			m_pCb = pCb;
			m_pInst = pPinst;
		}

		bool bValid(void)
		{
			return (m_pCb && m_pInst) ? true : false;
		}

		void call(uint32_t flag = 0)
		{
			if (!bValid())
				return;

			m_pCb(m_pInst, flag);
		}
	};

	struct WindowCV_Btn
	{
		string m_name = "btn";
		string m_label = "btn";
		bool m_bShow = false;
		vFloat4 m_vRegion = {0, 0, 1, 1};
		vInt4 m_vR;
		vInt3 m_colUp = {100, 100, 100};
		vInt3 m_colDown = {200, 200, 200};
		vInt3 m_colBorder = {255, 255, 255};
		vInt3 m_colFont = {255, 255, 255};
		int m_hFont = 25;
		cv::Point m_pT;
		uint64_t m_tDown;
		uint64_t m_tLongTap = 5 * SEC_2_USEC;

		bool m_bDown = false;
		WindowCV_CbBtn m_cb;

		void init(const cv::Size &s, void* pFont = NULL)
		{
			setScrSize(s);
			updateLabelPos(pFont);
		}

		void updateLabelPos(void* pFont = NULL)
		{
#ifdef USE_FREETYPE
			NULL_(pFont);

			int baseline = 0;
			cv::Ptr<freetype::FreeType2> pF = *(cv::Ptr<freetype::FreeType2>*)pFont;
			Size ts = pF->getTextSize(m_label,
									 m_hFont,
									 -1,
									 &baseline);

			m_pT.x = (m_vR.x + m_vR.z)/2 - ts.width/2;
			m_pT.y = (m_vR.y + m_vR.w)/2 - ts.height;
#endif
		}

		void setLabel(const string& label, void* pFont = NULL)
		{
			m_label = label;
//			updateLabelPos(pFont);						
		}

		void setVisible(bool bShow)
		{
			m_bShow = bShow;
		}

		void setCallback(CbWindowCVbtn pCb, void *pInst)
		{
			m_cb.add(pCb, pInst);
		}

		void setScrSize(const cv::Size &s)
		{
			m_vR.x = m_vRegion.x * s.width;
			m_vR.y = m_vRegion.y * s.height;
			m_vR.z = m_vRegion.z * s.width;
			m_vR.w = m_vRegion.w * s.height;
		}

		bool bInside(int x, int y)
		{
			IF_F(x < m_vR.x);
			IF_F(x > m_vR.z);
			IF_F(y < m_vR.y);
			IF_F(y > m_vR.w);

			return true;
		}

		void mouseDown(int x, int y)
		{
			IF_(!m_bShow);

			m_bDown = bInside(x, y);
			m_tDown = getApproxTbootUs();
		}

		void mouseMove(int x, int y)
		{
			IF_(!m_bShow);

			m_bDown &= bInside(x, y);
		}

		void mouseUp(int x, int y)
		{
			IF_(!m_bShow);

			m_bDown &= bInside(x, y);
			IF_(!m_bDown);

			uint32_t f = (getApproxTbootUs() - m_tDown > m_tLongTap)?1:0;
			m_cb.call(f);
			m_bDown = false;
		}

		void draw(Mat *pM, void* pFont = NULL)
		{
			IF_(!m_bShow);

			Rect r = bb2Rect(m_vR);
			Scalar colBg;
			if (m_bDown)
				colBg = {m_colDown.x, m_colDown.y, m_colDown.z};
			else
				colBg = {m_colUp.x, m_colUp.y, m_colUp.z};

			rectangle(*pM, r, colBg, FILLED);
			rectangle(*pM, r, Scalar(m_colBorder.x, m_colBorder.y, m_colBorder.z), 1);

#ifdef USE_FREETYPE
			if (pFont)
			{
				cv::Ptr<freetype::FreeType2> pF = *(cv::Ptr<freetype::FreeType2>*)pFont;
				pF->putText(*pM, m_label,
							   m_pT,
							   m_hFont,
							   Scalar(m_colFont.x, m_colFont.y, m_colFont.z),
							   -1,
							   cv::LINE_AA,
							   false);

				return;
			}
#endif

			putText(*pM, m_label,
					Point(m_vR.x, m_vR.y + 25),
					FONT_HERSHEY_SIMPLEX,
					1.0,
					Scalar(m_colFont.x, m_colFont.y, m_colFont.z),
					1,
					cv::LINE_AA);
		}
	};

	class _WindowCV : public _ModuleBase
	{
	public:
		_WindowCV();
		virtual ~_WindowCV();

		bool init(void *pKiss);
		bool start(void);

		Frame *getFrame(void);
		Frame *getNextFrame(void);
#ifdef USE_FREETYPE
		cv::Ptr<freetype::FreeType2> getFont(void);
#endif

		WindowCV_Btn* findBtn(const string& btnName);
		bool setCbBtn(const string &btnName, CbWindowCVbtn pCb, void *pInst);
		bool setBtnLabel(const string &btnName, const string &btnLabel);
		bool setBtnVisible(const string &btnName, bool bShow);
		void setBtnVisibleAll(bool bShow);

	protected:
		void updateWindow(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_WindowCV *)This)->update();
			return NULL;
		}

		static void sOnMouse(int event, int x, int y, int flags, void *userdata);
		void onMouse(int event, int x, int y, int flags);

	private:
		vector<BASE *> m_vpB;
		int m_waitKey;

		bool m_bShow;
		bool m_bFullScreen;

		tSwap<Frame> m_sF;
		vInt2 m_vSize;

		string m_gstOutput;
		VideoWriter m_gst;

		// UI
		vector<WindowCV_Btn> m_vBtn;
#ifdef USE_FREETYPE
		string m_font;
		cv::Ptr<freetype::FreeType2> m_pFT;
#endif
	};

}
#endif
