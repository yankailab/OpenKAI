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

		m_vRoi.init(0.0, 0.0, 1.0, 1.0);
		m_vClassRange.init(-INT_MAX, INT_MAX);

		m_bDrawClass = false;
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

		//general
		pK->v("minConfidence", &m_minConfidence);
		pK->v("rArea", &m_rArea);
		pK->v("rW", &m_rW);
		pK->v("rH", &m_rH);
		pK->v("vRoi", &m_vRoi);
		pK->v("vClassRange", &m_vClassRange);

		//draw
		pK->v("bDrawClass", &m_bDrawClass);
		pK->v("bDrawText", &m_bDrawText);
		pK->v("bDrawPos", &m_bDrawPos);

		m_sO.get()->init(pKiss);
		m_sO.next()->init(pKiss);
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
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			m_pT->autoFPSto();
		}
	}

	_Object *_Universe::add(_Object &o)
	{
		IF_N(o.area() < m_rArea.x || o.area() > m_rArea.y);
		IF_N(o.getWidth() < m_rW.x || o.getWidth() > m_rW.y);
		IF_N(o.getHeight() < m_rH.x || o.getHeight() > m_rH.y);
		IF_N(o.getTopClass() < m_vClassRange.x || o.getTopClass() > m_vClassRange.y);

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

	void _Universe::cvDraw(void *pWindow)
	{
#ifdef USE_OPENCV
		NULL_(pWindow);
		this->_ModuleBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getFrame();
		NULL_(pF);
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

			//bb
			Rect r = bb2Rect<vFloat4>(pO->getBB2Dscaled(pM->cols, pM->rows));
			rectangle(*pM, r, oCol, 1);

			//position
			if (m_bDrawPos)
			{
				putText(*pM, f2str(pO->getPos().z),
						Point(r.x + 15, r.y + 25),
						FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
			}

			//text
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

		//roi
		Rect roi = bb2Rect(bbScale(m_vRoi, pM->cols, pM->rows));
		rectangle(*pM, roi, Scalar(0, 255, 255), 1);

#endif
	}

}
