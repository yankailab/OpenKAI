/*
 * Window.cpp
 *
 *  Created on: May 24, 2022
 *      Author: Kai Yan
 */

#include "_UIbase.h"

namespace kai
{

	_UIbase::_UIbase()
	{
	}

	_UIbase::~_UIbase()
	{
	}

	bool _UIbase::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());

		return true;
	}

	bool _UIbase::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));
		m_pJ->erase("vBASE");

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _UIbase::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));
		const json &j = *m_pJ;

		string name;
		jKv(j, "RGBframeIn", name);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataStreamBase *>(pM->findDataStream(name)));
		IF_Le_F(!m_pRGBin, "RGBframeIn not found: " + name);

		return true;
	}

	Mat _UIbase::prepareImage(const Mat &image, const Vector2i &size)
	{
		if (size.x() <= 0 || size.y() <= 0)
		{
			return Mat();
		}

		const cv::Size outputSize(size.x(), size.y());
		const int channels = image.channels();
		if (image.empty() || image.dims != 2 || (channels != 1 && channels != 3 && channels != 4))
		{
			return Mat::zeros(outputSize, CV_8UC3);
		}

		Mat pixels;
		if (image.depth() == CV_8U)
		{
			pixels = image;
		}
		else
		{
			// Convert depth/thermal samples for display without changing the source.
			Mat floating;
			image.convertTo(floating, CV_32F);
			cv::patchNaNs(floating, 0.0);
			cv::normalize(floating, pixels, 0, 255, cv::NORM_MINMAX, CV_8U);
		}

		Mat bgr;
		if (channels == 1)
		{
			cv::cvtColor(pixels, bgr, cv::COLOR_GRAY2BGR);
		}
		else if (channels == 4)
		{
			cv::cvtColor(pixels, bgr, cv::COLOR_BGRA2BGR);
		}
		else
		{
			bgr = pixels;
		}

		if (bgr.size() == outputSize)
		{
			return bgr;
		}

		Mat resized;
		cv::resize(bgr, resized, outputSize);
		return resized;
	}

	bool _UIbase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _UIbase::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
		}
	}
}
