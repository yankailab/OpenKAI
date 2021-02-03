/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_Universe.h"

namespace kai
{

_Universe::_Universe()
{
	m_minConfidence = 0.0;
	m_rArea.init(-FLT_MAX, FLT_MAX);
	m_rW.init(-FLT_MAX, FLT_MAX);
	m_rH.init(-FLT_MAX, FLT_MAX);

	m_bbExpand = -1.0;
	m_bMerge = false;
	m_mergeOverlap = 0.8;
	m_vRoi.init(0.0, 0.0, 1.0, 1.0);

	m_bDrawStatistics = false;
	m_classLegendPos.init(25,100,15);
	m_bDrawClass = false;
	m_bDrawText = false;
	m_bDrawPos = false;

	clearObj();
}

_Universe::~_Universe()
{
}

bool _Universe::init(void* pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//general
	pK->v("minConfidence", &m_minConfidence);
	pK->v("rArea", &m_rArea);
	pK->v("rW", &m_rW);
	pK->v("rH", &m_rH);
	pK->v("bbExpand", &m_bbExpand);
	pK->v("bMerge", &m_bMerge);
	pK->v("mergeOverlap", &m_mergeOverlap);
	pK->v("vRoi", &m_vRoi);

	//draw
	pK->v("bDrawStatistics", &m_bDrawStatistics);
	pK->v("bDrawClass", &m_bDrawClass);
	pK->v("bDrawText", &m_bDrawText);
	pK->v("bDrawPos", &m_bDrawPos);

	m_pO[0].init(pKiss);
	m_pO[1].init(pKiss);
	clearObj();

	return true;
}

void _Universe::clearObj(void)
{
	m_iSwitch = 0;
	updateObj();
	m_pPrev->clear();
	m_pNext->clear();
}

void _Universe::updateObj(void)
{
	m_iSwitch = 1 - m_iSwitch;
	m_pPrev = &m_pO[m_iSwitch];
	m_pNext = &m_pO[1 - m_iSwitch];
	m_pNext->clear();
}

bool _Universe::start(void)
{
    IF_F(check()<0);
	return m_pT->start(getUpdate, this);
}

void _Universe::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		m_pT->autoFPSto();
	}
}

_Object* _Universe::add(_Object& o)
{
	IF_N(o.area() < m_rArea.x || o.area() > m_rArea.y);
	IF_N(o.getWidth() < m_rW.x || o.getWidth() > m_rW.y);
	IF_N(o.getHeight() < m_rH.x || o.getHeight() > m_rH.y);

	vFloat3 p = o.getPos();
	IF_N(p.x < m_vRoi.x);
	IF_N(p.x > m_vRoi.z);
	IF_N(p.y < m_vRoi.y);
	IF_N(p.y > m_vRoi.w);

//	if(m_bbScale > 0.0)
//	{
//		o.setBB2D(bbScale(o.getBB2D(), m_bbScale));
//	}

//	if(m_bMerge)
//	{
//		vFloat4 BB = o.getBB2D();
//
//		for(int i=0; i<m_pNext->size(); i++)
//		{
//			OBJECT* pO = &m_pNext->get(i);
//			if(!pO)break;
//			IF_CONT(pO->m_topClass != o.getTopClass());
//			IF_CONT(nIoU(BB, pO->m_bb) < m_mergeOverlap);
//
//			pO->m_bb.x = small(pNewO->m_bb.x, pO->m_bb.x);
//			pO->m_bb.y = small(pNewO->m_bb.y, pO->m_bb.y);
//			pO->m_bb.z = big(pNewO->m_bb.z, pO->m_bb.z);
//			pO->m_bb.w = big(pNewO->m_bb.w, pO->m_bb.w);
//			pO->m_topProb = big(pNewO->m_topProb, pO->m_topProb);
//			pO->m_mClass |= pNewO->m_mClass;
//
//			return pO;
//		}
//	}

	return m_pNext->add(o);
}

_Object* _Universe::get(int i)
{
	return m_pPrev->get(i);
}

int _Universe::size(void)
{
	return m_pPrev->size();
}

void _Universe::updateStatistics(void)
{
//	int i;
//	for(i=0; i<m_nClass; i++)
//		m_vClass[i].m_n = 0;
//
//	for(i=0; i<size(); i++)
//	{
//		_Object* pO = get(i);
//
//		if(!pO)break;
//		int iClass = pO->getTopClass();
//		IF_CONT(iClass >= m_nClass);
//		IF_CONT(iClass < 0);
//
//		m_vClass[iClass].m_n++;
//	}
}

void _Universe::draw(void)
{
	this->_ModuleBase::draw();

	addMsg("nObj=" + i2str(m_pPrev->size()), 1);

	_Object* pO;
	int i;

#ifdef USE_OPENCV
	if(checkWindow())
	{
		Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

		Scalar oCol;
		Scalar bCol = Scalar(100,100,100);
		int col;
		int colStep = 20;
		i = 0;
		while((pO = get(i++)) != NULL)
		{
			int iClass = pO->getTopClass();

			col = colStep * iClass;
			oCol = Scalar((col+85)%255, (col+170)%255, col) + bCol;

			//bb
			Rect r = bb2Rect<vFloat4>(pO->getBB2Dscaled(pMat->cols, pMat->rows));
			rectangle(*pMat, r, oCol, 1);

			//position
			if(m_bDrawPos)
			{
				putText(*pMat, f2str(pO->getPos().z),
						Point(r.x + 15, r.y + 25),
						FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
			}

			//class
	//		if(m_bDrawClass && iClass < m_nClass && iClass >= 0)
	//		{
	//			string oName = m_vClass[iClass].m_name;
	//			if (oName.length()>0)
	//			{
	//				putText(*pMat, oName,
	//						Point(r.x + 15, r.y + 50),
	//						FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
	//			}
	//		}

			//text
			if(m_bDrawText)
			{
				string oName = string(pO->getText());
				if (oName.length()>0)
				{
					putText(*pMat, oName,
							Point(r.x + 15, r.y + 50),
							FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
				}
			}
		}

		//roi
		Rect roi = bb2Rect(bbScale(m_vRoi, pMat->cols, pMat->rows));
		rectangle(*pMat, roi, Scalar(0,255,255), 1);

		IF_(!m_bDrawStatistics);
		updateStatistics();

	//	for(i=0; i<m_nClass; i++)
	//	{
	//		OBJ_CLASS* pC = &m_vClass[i];
	//		col = colStep * i;
	//		oCol = Scalar((col+85)%255, (col+170)%255, col) + bCol;
	//
	//		putText(*pMat, pC->m_name + ": " + i2str(pC->m_n),
	//				Point(m_classLegendPos.x, m_classLegendPos.y + i*m_classLegendPos.z),
	//				FONT_HERSHEY_SIMPLEX, 0.5, oCol, 1);
	//	}

	}
#endif

}

}
