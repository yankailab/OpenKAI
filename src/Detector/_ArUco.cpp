/*
 * _ArUco.cpp
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#include "_ArUco.h"

namespace kai
{

	_ArUco::_ArUco()
	{
		m_dict = aruco::DICT_4X4_50; // aruco::DICT_APRILTAG_16h5;
		m_realSize = 0.05;
		m_bPose = false;
	}

	_ArUco::~_ArUco()
	{
	}

	int _ArUco::init(void *pKiss)
	{
		CHECK_(this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v<uint8_t>("dict", &m_dict);
		m_pDict = cv::Ptr<cv::aruco::Dictionary>(new cv::aruco::Dictionary());
		*m_pDict = aruco::getPredefinedDictionary(m_dict);
		pK->v("realSize", &m_realSize);

		pK->v("bPose", &m_bPose);
		if (m_bPose)
		{
			string n;
			pK->v("fCalib", &n);
			readCamMatrices(n, &m_mC, &m_mD);
		}

		return OK_OK;
	}

	int _ArUco::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _ArUco::check(void)
	{
		NULL__(m_pV, OK_ERR_NULLPTR);
		NULL__(m_pU, OK_ERR_NULLPTR);

		return this->_DetectorBase::check();
	}

	void _ArUco::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			detect();

			ON_PAUSE;
		}
	}

	void _ArUco::detect(void)
	{
		IF_(check() != OK_OK);

		Mat m = *m_pV->getFrameRGB()->m();
		IF_(m.empty());

		vector<int> vID;
		vector<vector<Point2f>> vvCorner;
		aruco::detectMarkers(m, m_pDict, vvCorner, vID);
		vector<Vec3d> vvR, vvT;

		if (m_bPose)
		{
			if (m_mCscaled.empty())
			{
				scaleCamMatrices(cv::Size(m.cols, m.rows),
								 m_mC,
								 m_mD,
								 &m_mCscaled);
			}

			// pose
			aruco::estimatePoseSingleMarkers(vvCorner,
											 m_realSize,
											 m_mCscaled,
											 m_mD,
											 vvR,
											 vvT);
		}

		_Object o;
		float kx = 1.0 / (float)m.cols;
		float ky = 1.0 / (float)m.rows;

		for (unsigned int i = 0; i < vID.size(); i++)
		{
			o.clear();
			o.setType(obj_tag);
			o.setTopClass(vID[i], 1.0);

			if (m_bPose)
			{
				// pose
				Vec3d v;
				v = vvT[i];
				vFloat3 vP{v[0], v[1], v[2]};
				o.setPos(vP);
				v = vvR[i];
				vFloat3 vR{v[0], v[1], v[2]};
				o.setAttitude(vR);
			}

			// bbox
			Point2f pLT = vvCorner[i][0];
			Point2f pRT = vvCorner[i][1];
			Point2f pRB = vvCorner[i][2];
			Point2f pLB = vvCorner[i][3];

			vFloat2 pV[4];
			for (int j = 0; j < 4; j++)
			{
				pV[j].x = vvCorner[i][j].x;
				pV[j].y = vvCorner[i][j].y;
			}
			o.setVertices2D(pV, 4);

			// center position
			float dx = (float)(pLT.x + pRT.x + pRB.x + pLB.x) * 0.25;
			float dy = (float)(pLT.y + pRT.y + pRB.y + pLB.y) * 0.25;
			o.setPos(dx*kx, dy*ky, 0);

			// radius
			dx -= pLT.x;
			dy -= pLT.y;
			o.setDim(0,0,0, sqrt(dx*dx + dy*dy));	// vDim.w = radius

			// angle in deg
			dx = pLB.x - pLT.x;
			dy = pLB.y - pLT.y;
			o.setAttitude(-atan2(dx, dy) * RAD_2_DEG + 180.0, 0,0);	// roll

			m_pU->add(o);
			LOG_I("ID: " + i2str(o.getTopClass()));
		}

		m_pU->swap();
	}

	void _ArUco::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_DetectorBase::console(pConsole);
		IF_(check() != OK_OK);
	}

	void _ArUco::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_DetectorBase::draw(pFrame);
		IF_(check() != OK_OK);
	}

}
