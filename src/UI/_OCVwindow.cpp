/*
 * _OCVwindow.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#include "_OCVwindow.h"

namespace kai
{

	_OCVwindow::_OCVwindow()
	{
	}

	_OCVwindow::~_OCVwindow()
	{
	}

	bool _OCVwindow::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "bFullScreen", m_bFullScreen);
		jKv<int>(j, "vSize", m_vSize);

		IF_Le_F(m_vSize.x() <= 0 || m_vSize.y() <= 0, "Window size too small");

		string wn = this->getName();
		if (m_bFullScreen)
		{
			namedWindow(wn, WINDOW_NORMAL);
			setWindowProperty(wn, WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
		}
		else
		{
			namedWindow(wn, WINDOW_AUTOSIZE);
		}

		jKv<int>(j, "vSize", m_vSize);
		IF_Le_F(m_vSize.x() <= 0 || m_vSize.y() <= 0, "Invalid GStreamer output size");

		// optional Gstreamer output
		jKv(j, "gstOutput", m_gstOutput);
		if (!m_gstOutput.empty())
		{
			if (!m_gst.open(m_gstOutput,
							CAP_GSTREAMER,
							0,
							m_pT->getTargetFPS(),
							cv::Size(m_vSize.x(), m_vSize.y()),
							true))
			{
				LOG_E("Cannot open GStreamer output");
				return false;
			}
		}

		return true;
	}

	bool _OCVwindow::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		j["bFullScreen"] = m_bFullScreen;
		j["vSize"] = {m_vSize.x(), m_vSize.y()};
		j["gstOutput"] = m_gstOutput;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _OCVwindow::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));
		const json &j = *m_pJ;

		string n;

		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!m_pRGBin, "RGBframeIn not found: " + n);

		jKv(j, "BBoxStreamIn", n);
		m_pBBoxIn = dynamic_cast<BBoxStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));

		return true;
	}

	bool _OCVwindow::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _OCVwindow::check(void)
	{
		NULL_F(m_pRGBin);

		return _ModuleBase::check();
	}

	void _OCVwindow::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateWindow();
		}
	}

	void _OCVwindow::updateWindow(void)
	{
		IF_(!check());

		Mat mShow;
		m_pRGBin->get(mShow);
		IF_(mShow.empty());

		if (m_pBBoxIn)
		{
			vector<BBOX_OBJ> vBB;
			m_pBBoxIn->get(vBB);

			for (const BBOX_OBJ &bb : vBB)
			{
				const Vector4f bounds = bb.getBB2D();
				rectangle(mShow,
						  Point(cvRound(bounds.x()), cvRound(bounds.y())),
						  Point(cvRound(bounds.z()), cvRound(bounds.w())),
						  Scalar(0, 255, 0), 2, LINE_8);

				const string &name = bb.getTopClassName();
				if (!name.empty())
				{
					const cv::Size textSize = getTextSize(name, FONT_HERSHEY_SIMPLEX, 0.5, 1, nullptr);
					const Point textOrigin(std::max(0, cvRound(bounds.x()) + 2),
										   std::max(textSize.height, cvRound(bounds.y()) + textSize.height + 2));
					putText(mShow, name, textOrigin, FONT_HERSHEY_SIMPLEX, 0.5,
							Scalar(0, 255, 0), 1, LINE_8);
				}
			}
		}

		if (m_gst.isOpened())
		{
			m_gst << mShow;
		}

		// autoFPS() controls the refresh rate. Keep event handling short because
		// HighGUI serializes these calls across all preview windows.
		imshow(this->getName(), mShow);
		waitKey(1);
	}

}
