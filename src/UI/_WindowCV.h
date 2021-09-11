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

		void init(const cv::Size &s, cv::Ptr<freetype::FreeType2> pFont = NULL)
		{
			setScrSize(s);
			updateLabelPos(pFont);
		}

		void updateLabelPos(cv::Ptr<freetype::FreeType2> pFont = NULL)
		{
			NULL_(pFont);

			int baseline = 0;
			Size ts = pFont->getTextSize(m_label,
											 m_hFont,
											 -1,
											 &baseline);

			m_pT.x = (m_vR.x + m_vR.z)/2 - ts.width/2;
			m_pT.y = (m_vR.y + m_vR.w)/2 - ts.height;
		}

		void setLabel(const string& label, cv::Ptr<freetype::FreeType2> pFont = NULL)
		{
			m_label = label;
			updateLabelPos(pFont);						
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
			m_bDown = bInside(x, y);
			m_tDown = getApproxTbootUs();
		}

		void mouseMove(int x, int y)
		{
			m_bDown &= bInside(x, y);
		}

		void mouseUp(int x, int y)
		{
			m_bDown &= bInside(x, y);
			IF_(!m_bDown);

			uint32_t f = (getApproxTbootUs() - m_tDown > m_tLongTap)?1:0;
			m_cb.call(f);
			m_bDown = false;
		}

		void draw(Mat *pM, cv::Ptr<freetype::FreeType2> pFont = NULL)
		{
			Rect r = bb2Rect(m_vR);
			Scalar colBg;
			if (m_bDown)
				colBg = {m_colDown.x, m_colDown.y, m_colDown.z};
			else
				colBg = {m_colUp.x, m_colUp.y, m_colUp.z};

			rectangle(*pM, r, colBg, FILLED);
			rectangle(*pM, r, Scalar(m_colBorder.x, m_colBorder.y, m_colBorder.z), 1);

			if (pFont)
			{
				pFont->putText(*pM, m_label,
							   m_pT,
							   m_hFont,
							   Scalar(m_colFont.x, m_colFont.y, m_colFont.z),
							   -1,
							   cv::LINE_AA,
							   false);

				return;
			}

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
		cv::Ptr<freetype::FreeType2> getFont(void);

		WindowCV_Btn* findBtn(const string& btnName);
		bool setCbBtn(const string &btnName, CbWindowCVbtn pCb, void *pInst);
		bool setBtnLabel(const string &btnName, const string &btnLabel);

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

	public:
		vector<BASE *> m_vpB;
		int m_waitKey;

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

		// UI
		vector<WindowCV_Btn> m_vBtn;
		cv::Ptr<freetype::FreeType2> m_pFT;
		string m_font;
	};

}
#endif
#endif
