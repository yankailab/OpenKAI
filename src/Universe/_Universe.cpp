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
		m_vArea.set(-FLT_MAX, FLT_MAX);
		m_vW.set(-FLT_MAX, FLT_MAX);
		m_vH.set(-FLT_MAX, FLT_MAX);

		m_vRoi.set(0.0, 0.0, 1.0, 1.0);
		m_vClassRange.set(-INT_MAX, INT_MAX);

		m_bDrawText = false;
		m_bDrawPos = false;

		clear();
	}

	_Universe::~_Universe()
	{
	}

	bool _Universe::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		// general
		pK->v("minConfidence", &m_minConfidence);
		pK->v("vArea", &m_vArea);
		pK->v("vW", &m_vW);
		pK->v("vH", &m_vH);
		pK->v("vRoi", &m_vRoi);
		pK->v("vClassRange", &m_vClassRange);

		// draw
		pK->v("bDrawText", &m_bDrawText);
		pK->v("bDrawPos", &m_bDrawPos);

		// buffer
		int nB = 16;
		pK->v("nBuf", &nB);
		m_sO.get()->init(nB);
		m_sO.next()->init(nB);
		clear();

		return true;
	}

	void _Universe::clear(void)
	{
		m_sO.get()->clear();
		m_sO.next()->clear();
	}

	void _Universe::swap(void)
	{
		m_sO.swap();
		m_sO.next()->clear();
	}

	bool _Universe::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Universe::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			m_pT->autoFPSto();
		}
	}

	_Object *_Universe::add(_Object &o)
	{
		IF_N(!m_vArea.bInside(o.area()));
		IF_N(!m_vW.bInside(o.getWidth()));
		IF_N(!m_vH.bInside(o.getHeight()));
		IF_N(!m_vClassRange.bInside(o.getTopClass()));

		vFloat3 p = o.getPos();
		IF_N(p.x < m_vRoi.x);
		IF_N(p.x > m_vRoi.z);
		IF_N(p.y < m_vRoi.y);
		IF_N(p.y > m_vRoi.w);

		return m_sO.next()->add(o);
	}

	_Object *_Universe::get(int i)
	{
		return m_sO.get()->get(i);
	}

	int _Universe::size(void)
	{
		return m_sO.get()->size();
	}

	void _Universe::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		((_Console *)pConsole)->addMsg("nObj=" + i2str(m_sO.get()->size()), 1);
	}

	void _Universe::draw(void *pFrame)
	{
#ifdef USE_OPENCV
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		Frame *pF = (Frame *)pFrame;
		Mat *pM = pF->m();
		IF_(pM->empty());

		_Object *pO;
		Scalar oCol;
		Scalar bCol = Scalar(100, 100, 100);
		int col;
		int colStep = 20;
		int i = 0;
		while ((pO = get(i++)) != NULL)
		{
			int iClass = pO->getTopClass();

			col = colStep * iClass;
			oCol = Scalar((col + 85) % 255, (col + 170) % 255, col) + bCol;

			// bb
			Rect r = bb2Rect<vFloat4>(pO->getBB2Dscaled(pM->cols, pM->rows));
			rectangle(*pM, r, oCol, 1);

			// position
			if (m_bDrawPos)
			{
				putText(*pM, f2str(pO->getPos().z),
						Point(r.x + 15, r.y + 25),
						FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
			}

			// text
			if (m_bDrawText)
			{
				string oName = string(pO->getText());
				if (oName.length() > 0)
				{
					putText(*pM, oName,
							Point(r.x + 15, r.y + 50),
							FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
				}
			}
		}

		// roi
		Rect roi = bb2Rect(bbScale(m_vRoi, pM->cols, pM->rows));
		rectangle(*pM, roi, Scalar(0, 255, 255), 1);
#endif
	}

}
