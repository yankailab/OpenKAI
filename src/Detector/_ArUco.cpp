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
	}

	_ArUco::~_ArUco()
	{
	}

	bool _ArUco::loadConfig(void)
	{
		IF_F(!this->_DetectorBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "dict", m_dict);
		m_dictionary = aruco::getPredefinedDictionary(m_dict);
		m_detector.setDictionary(m_dictionary);
		jKv(j, "realSize", m_realSize);

		return true;
	}

	bool _ArUco::saveConfig(bool bExport)
	{
		IF_F(!_DetectorBase::saveConfig(false));

		json &j = *m_pJ;
		j["dict"] = m_dict;
		j["realSize"] = m_realSize;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _ArUco::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _ArUco::check(void)
	{
		NULL_F(m_pRGBin);

		return this->_DetectorBase::check();
	}

	void _ArUco::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			detect();
		}
	}

	void _ArUco::detect(void)
	{
		IF_(!check());

		Mat m;
		const uint64_t tStamp = m_pRGBin->get(m);
		IF_(tStamp == m_tLastInput);
		m_tLastInput = tStamp;
		IF_(m.empty());
		m_pBBout->setContainerDim(Vector3f(m.cols, m.rows, 0));
		vector<int> vID;
		vector<vector<Point2f>> vvCorner;
		m_detector.detectMarkers(m, vvCorner, vID);
		vector<Vec3d> vvR, vvT;

		vector<BBOX_OBJ> vBB;
		for (size_t i = 0; i < vID.size(); i++)
		{
			// bbox
			Point2f pLT = vvCorner[i][0];
			Point2f pRT = vvCorner[i][1];
			Point2f pRB = vvCorner[i][2];
			Point2f pLB = vvCorner[i][3];

			// center position
			float cx = (float)(pLT.x + pRT.x + pRB.x + pLB.x) * 0.25;
			float cy = (float)(pLT.y + pRT.y + pRB.y + pLB.y) * 0.25;

			// radius
			float dx = cx - pLT.x;
			float dy = cy - pLT.y;
			float r = sqrt(dx * dx + dy * dy);

			// angle in deg
			dx = pLB.x - pLT.x;
			dy = pLB.y - pLT.y;
			float a = -atan2(dx, dy) * RAD_2_DEG + 180.0;

			// vertices
			Vector2f pV[4];
			for (int j = 0; j < 4; j++)
			{
				pV[j].x() = vvCorner[i][j].x;
				pV[j].y() = vvCorner[i][j].y;
			}

			BBOX_OBJ bb;
			bb.setType(obj_tag);
			bb.setPos(Vector3f(cx, cy, a));
			bb.setDim(Vector3f(r, 0, 0));
			bb.addClass(vID[i]);

			vBB.push_back(bb);
		}

		m_pBBout->add(vBB, tStamp);
	}

	void _ArUco::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_DetectorBase::console(pConsole);
		IF_(!check());
	}

}
