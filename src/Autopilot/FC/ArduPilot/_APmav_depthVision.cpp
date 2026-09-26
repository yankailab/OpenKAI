#include "_APmav_depthVision.h"

namespace kai
{

	_APmav_depthVision::_APmav_depthVision()
	{
	}

	_APmav_depthVision::~_APmav_depthVision()
	{
	}

	bool _APmav_depthVision::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		m_nROI = 0;
		const json *pJS = jK(j, "sections");
		IF__(!pJS || !pJS->is_object(), true);
		const json &jS = *pJS;

		for (auto it = jS.begin(); it != jS.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			IF_F(m_nROI >= N_DEPTH_ROI);

			DEPTH_ROI *pR = &m_pROI[m_nROI];
			pR->init();
			pR->m_configKey = it.key();
			jKv(Ji, "orientation", pR->m_orientation);
			jKv(Ji, "l", pR->m_roi.x());
			jKv(Ji, "t", pR->m_roi.y());
			jKv(Ji, "r", pR->m_roi.z());
			jKv(Ji, "b", pR->m_roi.w());

			m_nROI++;
		}

		return true;
	}

	bool _APmav_depthVision::saveConfig(bool bExport)
	{
		if (!_ModuleBase::saveConfig(false))
		{
			return false;
		}

		json &j = *m_pJ;
		json &sections = j["sections"];
		if (!sections.is_object())
		{
			sections = json::object();
		}
		for (int i = 0; i < m_nROI; ++i)
		{
			DEPTH_ROI &roi = m_pROI[i];
			if (roi.m_configKey.empty())
			{
				roi.m_configKey = std::to_string(i);
				while (sections.contains(roi.m_configKey))
				{
					roi.m_configKey += "_";
				}
			}
			json &value = sections[roi.m_configKey];
			if (!value.is_object())
			{
				value = json::object();
			}
			value["orientation"] = roi.m_orientation;
			value["l"] = roi.m_roi[0];
			value["t"] = roi.m_roi[1];
			value["r"] = roi.m_roi[2];
			value["b"] = roi.m_roi[3];
		}

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _APmav_depthVision::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		const json &j = *m_pJ;

		string n;

		n = "";
		jKv(j, "APmavlink_base", n);
		m_pAP = (_APmav_base *)(m_pM->findModule(n));
		NULL_F(m_pAP);

		n = "";
		jKv(j, "_RGBDbase", n);
		m_pDV = (_RGBDbase *)(m_pM->findModule(n));
		NULL_F(m_pDV);

		return true;
	}

	void _APmav_depthVision::update(void)
	{
		NULL_(m_pAP);
		NULL_(m_pAP->getMavlink());
		_Mavlink *pMavlink = m_pAP->getMavlink();
		NULL_(m_pDV);

		Vector2f range = m_pDV->getDepthRange();
		mavlink_distance_sensor_t D;

		for (int i = 0; i < m_nROI; i++)
		{
			DEPTH_ROI *pR = &m_pROI[i];

			// float d = m_pDV->d(pR->m_roi);
			// if (d <= range.x())
			// 	d = range.y();
			// if (d > range.y())
			// 	d = range.y();
			// pR->m_minD = d;

			D.type = 0;
			D.max_distance = (uint16_t)(range.y() * 100); // unit: centimeters
			D.min_distance = (uint16_t)(range.x() * 100);
			D.current_distance = (uint16_t)(pR->m_minD * 100);
			D.orientation = pR->m_orientation;
			D.covariance = 255;

			pMavlink->distanceSensor(D);
			LOG_I("orient: " + i2str(pR->m_orientation) + " minD: " + f2str(pR->m_minD));
		}
	}

	void _APmav_depthVision::draw(void *pMat)
	{
		NULL_(pMat);
		this->_ModuleBase::draw(pMat);
		IF_(!check());

		Mat *pM = static_cast<Mat *>(pMat);

		NULL_(m_pDV);

		for (int i = 0; i < m_nROI; i++)
		{
			DEPTH_ROI *pR = &m_pROI[i];
			Vector4f roi = pR->m_roi;
//			float d = m_pDV->d(roi);

			Rect r;
			r.x = roi.x() * pM->cols;
			r.y = roi.y() * pM->rows;
			r.width = roi.z() * pM->cols - r.x;
			r.height = roi.w() * pM->rows - r.y;
			rectangle(*pM, r, Scalar(0, 255, 0), 1);

			// putText(*pM, f2str(d),
			// 		Point(r.x + 15, r.y + 25),
			// 		FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 255), 1);
		}
	}

}
