/*
 * _ArUco.cpp
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#include "_ArUco.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

_ArUco::_ArUco()
{
	m_dict = aruco::DICT_4X4_50;//aruco::DICT_APRILTAG_16h5;
	m_minArea = -DBL_MAX;
	m_maxArea = DBL_MAX;
	m_maxW = DBL_MAX;
	m_maxH = DBL_MAX;
}

_ArUco::~_ArUco()
{
}

bool _ArUco::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v<uint8_t>("dict", &m_dict);
	m_pDict = aruco::getPredefinedDictionary(m_dict);

	return true;
}

bool _ArUco::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _ArUco::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();
		updateObj();

		if(m_bGoSleep)
		{
			m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

void _ArUco::detect(void)
{
	NULL_(m_pV);
	Mat m = *m_pV->BGR()->m();
	IF_(m.empty());

	vInt2 cs;
	cs.x = m.cols;
	cs.y = m.rows;
	float bW = 1.0/(float)m.cols;
	float bH = 1.0/(float)m.rows;

    std::vector<int> vID;
    std::vector<std::vector<cv::Point2f> > vvCorner;
    cv::aruco::detectMarkers(m, m_pDict, vvCorner, vID);

	OBJECT o;
	float dx,dy;

	for (unsigned int i = 0; i < vID.size(); i++)
	{
		o.init();
		o.m_tStamp = m_tStamp;
		o.setTopClass(vID[i],1.0);

		Point2f pLT = vvCorner[i][0];
		Point2f pRT = vvCorner[i][1];
		Point2f pRB = vvCorner[i][2];
		Point2f pLB = vvCorner[i][3];

		// bbox
		vFloat2 pV[4];
		for (int j = 0; j < 4; j++)
		{
			pV[j].x = vvCorner[i][j].x;
			pV[j].y = vvCorner[i][j].y;
		}
		o.setVertices(pV,4);
		o.normalizeBB(cs);

		// distance
		if(m_pDV)
			o.m_dist = m_pDV->d(&o.m_bb);

		// center position
		dx = (float)(pLT.x + pRT.x + pRB.x + pLB.x)*0.25;
		dy = (float)(pLT.y + pRT.y + pRB.y + pLB.y)*0.25;
		o.m_c.x = dx * bW;
		o.m_c.y = dy * bH;

		// radius
		dx -= pLT.x;
		dy -= pLT.y;
		o.m_r = sqrt(dx*dx + dy*dy);

		// angle in deg
		dx = pLB.x - pLT.x;
		dy = pLB.y - pLT.y;
		o.m_angle = -atan2(dx,dy) * RAD_2_DEG + 180.0;

		add(&o);
		LOG_I("ID: "+ i2str(o.m_topClass));
	}
}

void _ArUco::draw(void)
{
	this->_DetectorBase::draw();

	string msg = "| ";
	OBJECT* pO;
	int i=0;
	while((pO = at(i++)) != NULL)
	{
		msg += i2str(pO->m_topClass) + "("+ f2str(pO->m_dist) +") | ";
	}
	addMsg(msg, 1);

	IF_(this->size() <= 0);

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	i=0;
	while((pO = at(i++)) != NULL)
	{
		Point pCenter = Point(pO->m_c.x * pMat->cols,
							  pO->m_c.y * pMat->rows);
		circle(*pMat, pCenter, pO->m_r, Scalar(255, 255, 0), 2);

		putText(*pMat, "iTag=" + i2str(pO->m_topClass) + ", angle=" + i2str(pO->m_angle),
				pCenter,
				FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 0);

		double rad = -pO->m_angle * DEG_2_RAD;
		Point pD = Point(pO->m_r*sin(rad), pO->m_r*cos(rad));
		line(*pMat, pCenter + pD, pCenter - pD, Scalar(0, 0, 255), 2);
	}
}

}
#endif
