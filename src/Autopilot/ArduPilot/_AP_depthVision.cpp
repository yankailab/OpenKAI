#include "../ArduPilot/_AP_depthVision.h"

#ifdef USE_OPENCV

namespace kai
{

	_AP_depthVision::_AP_depthVision()
	{
		m_pAP = NULL;
		m_pDV = NULL;
		m_nROI = 0;
	}

	_AP_depthVision::~_AP_depthVision()
	{
	}

	bool _AP_depthVision::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		//link
		string n;

		n = "";
		F_INFO(pK->v("APcopter_base", &n));
		m_pAP = (_AP_base *)(pK->getInst(n));

		n = "";
		F_INFO(pK->v("_DepthVisionBase", &n));
		m_pDV = (_DepthVisionBase *)(pK->getInst(n));
		IF_Fl(!m_pDV, n + " not found");

		m_nROI = 0;
		while (1)
		{
			IF_F(m_nROI >= N_DEPTH_ROI);
			Kiss *pKs = pK->child(m_nROI);
			if (pKs->empty())
				break;

			DEPTH_ROI *pR = &m_pROI[m_nROI];
			pR->init();
			F_ERROR_F(pKs->v("orientation", (int *)&pR->m_orientation));
			F_ERROR_F(pKs->v("l", &pR->m_roi.x));
			F_ERROR_F(pKs->v("t", &pR->m_roi.y));
			F_ERROR_F(pKs->v("r", &pR->m_roi.z));
			F_ERROR_F(pKs->v("b", &pR->m_roi.w));

			m_nROI++;
		}

		return true;
	}

	void _AP_depthVision::update(void)
	{
		this->_StateBase::update();

		NULL_(m_pAP);
		NULL_(m_pAP->m_pMav);
		_Mavlink *pMavlink = m_pAP->m_pMav;
		NULL_(m_pDV);

		vFloat2 range = m_pDV->m_vRange;
		mavlink_distance_sensor_t D;

		for (int i = 0; i < m_nROI; i++)
		{
			DEPTH_ROI *pR = &m_pROI[i];

			float d = m_pDV->d(&pR->m_roi);
			if (d <= range.x)
				d = range.y;
			if (d > range.y)
				d = range.y;
			pR->m_minD = d;

			D.type = 0;
			D.max_distance = (uint16_t)(range.y * 100); //unit: centimeters
			D.min_distance = (uint16_t)(range.x * 100);
			D.current_distance = (uint16_t)(pR->m_minD * 100);
			D.orientation = pR->m_orientation;
			D.covariance = 255;

			pMavlink->distanceSensor(D);
			LOG_I("orient: " + i2str(pR->m_orientation) + " minD: " + f2str(pR->m_minD));
		}
	}

	void _AP_depthVision::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		this->_StateBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getNextFrame();
		NULL_(pF);
		Mat *pM = pF->m();
		IF_(pM->empty());

		NULL_(m_pDV);

		for (int i = 0; i < m_nROI; i++)
		{
			DEPTH_ROI *pR = &m_pROI[i];
			vFloat4 roi = pR->m_roi;
			float d = m_pDV->d(&roi);

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
#endif
