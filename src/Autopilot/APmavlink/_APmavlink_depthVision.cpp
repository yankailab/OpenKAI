#include "_APmavlink_depthVision.h"

namespace kai
{

	_APmavlink_depthVision::_APmavlink_depthVision()
	{
		m_pAP = nullptr;
		m_pDV = nullptr;
		m_nROI = 0;
	}

	_APmavlink_depthVision::~_APmavlink_depthVision()
	{
	}

	bool _APmavlink_depthVision::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		const json &js = j.at("sections");
		IF__(!js.is_object(), true);

		m_nROI = 0;
		for (auto it = js.begin(); it != js.end(); it++)
		{
			const json &ji = it.value();
			IF_CONT(!ji.is_object());

			IF_F(m_nROI >= N_DEPTH_ROI);

			DEPTH_ROI *pR = &m_pROI[m_nROI];
			pR->init();
			pR->m_orientation = ji.value("orientation", pR->m_orientation);
			pR->m_roi.x = ji.value("l", pR->m_roi.x);
			pR->m_roi.y = ji.value("t", pR->m_roi.y);
			pR->m_roi.z = ji.value("r", pR->m_roi.z);
			pR->m_roi.w = ji.value("b", pR->m_roi.w);

			m_nROI++;
		}

		return true;
	}

	bool _APmavlink_depthVision::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n;

		n = j.value("APmavlink_base", "");
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		n = j.value("_RGBDbase", "");
		m_pDV = (_RGBDbase *)(pM->findModule(n));
		NULL_F(m_pDV);

		return true;
	}

	void _APmavlink_depthVision::update(void)
	{
		NULL_(m_pAP);
		NULL_(m_pAP->getMavlink());
		_Mavlink *pMavlink = m_pAP->getMavlink();
		NULL_(m_pDV);

		vFloat2 range = m_pDV->getRangeD();
		mavlink_distance_sensor_t D;

		for (int i = 0; i < m_nROI; i++)
		{
			DEPTH_ROI *pR = &m_pROI[i];

			float d = m_pDV->d(pR->m_roi);
			if (d <= range.x)
				d = range.y;
			if (d > range.y)
				d = range.y;
			pR->m_minD = d;

			D.type = 0;
			D.max_distance = (uint16_t)(range.y * 100); // unit: centimeters
			D.min_distance = (uint16_t)(range.x * 100);
			D.current_distance = (uint16_t)(pR->m_minD * 100);
			D.orientation = pR->m_orientation;
			D.covariance = 255;

			pMavlink->distanceSensor(D);
			LOG_I("orient: " + i2str(pR->m_orientation) + " minD: " + f2str(pR->m_minD));
		}
	}

	void _APmavlink_depthVision::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(!check());

		Frame *pF = (Frame *)pFrame;
		Mat *pM = pF->m();

		NULL_(m_pDV);

		for (int i = 0; i < m_nROI; i++)
		{
			DEPTH_ROI *pR = &m_pROI[i];
			vFloat4 roi = pR->m_roi;
			float d = m_pDV->d(roi);

			Rect r;
			r.x = roi.x * pM->cols;
			r.y = roi.y * pM->rows;
			r.width = roi.z * pM->cols - r.x;
			r.height = roi.w * pM->rows - r.y;
			rectangle(*pM, r, Scalar(0, 255, 0), 1);

			putText(*pM, f2str(d),
					Point(r.x + 15, r.y + 25),
					FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 255), 1);
		}
	}

}
