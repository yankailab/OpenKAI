/*
 * _ArUco.cpp
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#ifdef USE_OPENCV
#ifdef USE_OPENCV_CONTRIB
#include "_ArUco.h"

namespace kai
{

	_ArUco::_ArUco()
	{
		m_dict = aruco::DICT_4X4_50; //aruco::DICT_APRILTAG_16h5;
		m_realSize = 0.05;
	}

	_ArUco::~_ArUco()
	{
	}

	bool _ArUco::init(void *pKiss)
	{
		IF_F(!this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v<uint8_t>("dict", &m_dict);
		m_pDict = aruco::getPredefinedDictionary(m_dict);

		pK->v("realSize", &m_realSize);

		return true;
	}

	bool _ArUco::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ArUco::check(void)
	{
		NULL__(m_pV, -1);
		NULL__(m_pU, -1);

		return this->_DetectorBase::check();
	}

	void _ArUco::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (check() >= 0)
			{
				detect();

				if (m_pT->bGoSleep())
					m_pU->clear();
			}

			m_pT->autoFPSto();
		}
	}

	void _ArUco::detect(void)
	{
		Mat m = *m_pV->BGR()->m();
		IF_(m.empty());

		vector<int> vID;
		vector<vector<Point2f>> vvCorner;
		aruco::detectMarkers(m, m_pDict, vvCorner, vID);

		// pose
		vector<Vec3d> vvR, vvT;
        aruco::estimatePoseSingleMarkers(vvCorner, 
										m_realSize, 
										m_pV->mCscaled(), 
										m_pV->mD(), 
										vvR, 
										vvT);

		_Object o;
		float dx, dy;
		float kx = 1.0 / (float)m.cols;
		float ky = 1.0 / (float)m.rows;

		for (unsigned int i = 0; i < vID.size(); i++)
		{
			o.init();
			o.setTopClass(vID[i], 1.0);

			// pose
			Vec3d v;
			v = vvT[i];
			vFloat3 vP{v[0], v[1], v[2]};
			o.setPos(vP);
			v = vvR[i];
			vFloat3 vR{v[0], v[1], v[2]};
			o.setAttitude(vR);

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
			o.scale(kx, ky);

			// center position
			dx = (float)(pLT.x + pRT.x + pRB.x + pLB.x) * 0.25;
			dy = (float)(pLT.y + pRT.y + pRB.y + pLB.y) * 0.25;
			o.setX(dx * kx);
			o.setY(dy * ky);

			// radius
			dx -= pLT.x;
			dy -= pLT.y;
			o.setRadius(sqrt(dx * dx + dy * dy));

			// angle in deg
			dx = pLB.x - pLT.x;
			dy = pLB.y - pLT.y;
			o.setRoll(-atan2(dx, dy) * RAD_2_DEG + 180.0);

			m_pU->add(o);
			LOG_I("ID: " + i2str(o.getTopClass()));
		}

		m_pU->swap();
	}

	void _ArUco::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(check() < 0);
		this->_DetectorBase::console(pConsole);

		_Object *pO;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			string msg = "id=" + i2str(pO->getTopClass()) +
						 ", pos=(" + f2str(pO->getX()) + ", " + f2str(pO->getY()) + ", " + f2str(pO->getZ()) +
						 ")";
			((_Console *)pConsole)->addMsg(msg, 1);
		}
	}

	void _ArUco::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		this->_DetectorBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getNextFrame();
		NULL_(pF);
		Mat *pM = pF->m();
		IF_(pM->empty());

		IF_(m_pU->size() <= 0);
		int i = 0;
		_Object* pO;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			Point pCenter = Point(pO->getX() * pM->cols,
								  pO->getY() * pM->rows);
			circle(*pM, pCenter, pO->getRadius(), Scalar(255, 255, 0), 2);

			putText(*pM, "iTag=" + i2str(pO->getTopClass()) + ", angle=" + i2str(pO->getRoll()),
					pCenter,
					FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 0);

			double rad = -pO->getRoll() * DEG_2_RAD;
			Point pD = Point(pO->getRadius() * sin(rad), pO->getRadius() * cos(rad));
			line(*pM, pCenter + pD, pCenter - pD, Scalar(0, 0, 255), 2);
		}
	}

}
#endif
#endif
