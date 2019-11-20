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
	m_pVision = NULL;
	m_dict = aruco::DICT_4X4_50;//DICT_APRILTAG_16h5;
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

	string iName = "";
	F_ERROR_F(pK->v("_VisionBase",&iName));
	m_pVision = (_VisionBase*)(pK->root()->getChildInst(iName));

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
	NULL_(m_pVision);
	Mat m = *m_pVision->BGR()->m();
	IF_(m.empty());

	vInt2 cs;
	cs.x = m.cols;
	cs.y = m.rows;
	double bW = 1.0/(double)m.cols;
	double bH = 1.0/(double)m.rows;

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
//		for (int p = 0; p < 4; p++)
//		{
//			o.m_pV[p].x = vvCorner[i][p].x * bW;
//			o.m_pV[p].y = vvCorner[i][p].y * bH;
//		}
//		o.m_nV = 4;
//		o.updateBB(cs);

		// center position
		dx = (double)(pLT.x + pRT.x + pRB.x + pLB.x)*0.25;
		dy = (double)(pLT.y + pRT.y + pRB.y + pLB.y)*0.25;
		o.m_c.x = dx * bW;
		o.m_c.y = dy * bH;

		// radius
		dx -= pLT.x;
		dy -= pLT.y;
		o.m_r = sqrt(dx*dx + dy*dy);

		// angle in deg
		dx = pLB.x - pLT.x;
		dy = pLB.y - pLT.y;
		o.m_angle = -atan2(dx,dy) * RAD_DEG + 180.0;

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
		msg += i2str(pO->m_topClass) + " | ";
	}
	addMsg(msg);

	msg = "nTag: " + i2str(this->size());
	addMsg(msg);
	IF_(this->size() <= 0);

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	i=0;
	while((pO = at(i++)) != NULL)
	{
		Point pCenter = Point(pO->m_c.x * pMat->cols,
							  pO->m_c.y * pMat->rows);
		circle(*pMat, pCenter, pO->m_r, Scalar(0, 255, 0), 2);

		putText(*pMat, i2str(pO->m_topClass) + " / " + i2str(pO->m_angle),
				pCenter,
				FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0), 2);

		double rad = -pO->m_angle * DEG_RAD;
		Point pD = Point(pO->m_r*sin(rad), pO->m_r*cos(rad));
		line(*pMat, pCenter + pD, pCenter - pD, Scalar(0, 0, 255), 2);
	}
}

}
#endif
