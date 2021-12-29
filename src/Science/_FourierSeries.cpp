/*
 * _FourierSeries.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_FourierSeries.h"

namespace kai
{
	_FourierSeries* g_pFourier = NULL;

	_FourierSeries::_FourierSeries()
	{
		m_S = 1;
		m_M = 1;
		m_N = 1;
		m_w = 0.01;
		m_nDigit = 10;
		m_vOrigin.init(0);
		m_vRangeX.init(-1,1);
		m_vRangeY.init(-1,1);
		g_pFourier = this;
	}

	_FourierSeries::~_FourierSeries()
	{
	}

	bool _FourierSeries::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("S", &m_S);
		pK->v("M", &m_M);
		pK->v("N", &m_N);
		pK->v("w", &m_w);
		pK->v("vOrigin", &m_vOrigin);
		pK->v("vRangeX", &m_vRangeX);
		pK->v("vRangeY", &m_vRangeY);
		pK->v("nDigit", &m_nDigit);

		return true;
	}

	bool _FourierSeries::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _FourierSeries::update(void)
	{
		mglFLTK gr(cbGraph, this->getName()->c_str());
		gr.Run();
	}

	void _FourierSeries::updateSeries(void)
	{
		double a0 = 3.0*m_w/CV_PI;
		m_fy = lf2str(a0, m_nDigit);

		for(int i=1; i<=m_S; i++)
		{
			double a = (2.0/CV_PI)*(1.0/(double)i)*sin((double)i*m_w);

			m_fy += "+" + lf2str(a, m_nDigit);
			m_fy += "*cos(" + i2str(i) + "*x)";
		}

		for(int i=1; i<=m_S; i++)
		{
			double a = (2.0/CV_PI)*(1.0/(double)i)*sin((double)i*m_w);

			m_fy += "+" + lf2str(a, m_nDigit);
			m_fy += "*cos(" + i2str(i);
			m_fy += "*(x-2*" + lf2str(m_w, m_nDigit) + "))";
		}

		for(int i=1; i<=m_S; i++)
		{
			double a = (2.0/CV_PI)*(1.0/(double)i)*sin((double)i*m_w);

			m_fy += "+" + lf2str(a, m_nDigit);
			m_fy += "*cos(" + i2str(i);
			m_fy += "*(x-4*" + lf2str(m_w, m_nDigit) + "))";
		}
	}

	int _FourierSeries::cbGraph(mglGraph *pGr)
	{
		g_pFourier->updateSeries();

		pGr->Title("Fourier series");
		pGr->SetOrigin(g_pFourier->m_vOrigin.x,
					   g_pFourier->m_vOrigin.y);
		pGr->SetRanges(g_pFourier->m_vRangeX.x,
					   g_pFourier->m_vRangeX.y,
					   g_pFourier->m_vRangeY.x,
					   g_pFourier->m_vRangeY.y);
		pGr->FPlot(g_pFourier->m_fy.c_str(), "r-4");
		pGr->Axis();
		pGr->Grid();
		return 0;
	}

}
