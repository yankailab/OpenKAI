/*
 * _PCtransform.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCtransform.h"

namespace kai
{

	_PCtransform::_PCtransform()
	{
		m_mTt = Matrix4d::Identity();
		m_paramKiss = "";
	}

	_PCtransform::~_PCtransform()
	{
	}

	int _PCtransform::init(void *pKiss)
	{
		CHECK_(_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		// read from external kiss file if there is one
		pK->v("paramKiss", &m_paramKiss);
		IF__(m_paramKiss.empty(), OK_OK);

		string s;
		if (!readFile(m_paramKiss, &s))
		{
			LOG_I("Cannot open: " + m_paramKiss);
			return OK_OK;
		}

		IF__(s.empty(), OK_OK);

		Kiss *pKf = new Kiss();
		if (pKf->parse(s))
		{
			pK = pKf->child("transform");
			pK->v("vT", &m_vT);
			pK->v("vR", &m_vR);
		}
		delete pKf;

		return OK_OK;
	}

	int _PCtransform::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _PCtransform::check(void)
	{
//		NULL__(m_pInCtx.m_pPCB, -1);

		return this->_PCframe::check();
	}

	void _PCtransform::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateTransform();

		}
	}

	void _PCtransform::updateTransform(void)
	{
		IF_(check() != OK_OK);

//		readPC(m_pInCtx.m_pPCB);
		m_sPC.next()->Transform(m_mTt);
		swapBuffer();
	}

	void _PCtransform::setTranslationMatrix(Matrix4d_u &mTt)
	{
		m_mTt = mTt * m_mT;
	}

	void _PCtransform::saveParamKiss(void)
	{
		IF_(m_paramKiss.empty());

		picojson::object o;
		o.insert(make_pair("name", "transform"));

		picojson::array vT;
		vT.push_back(value(m_vT.x));
		vT.push_back(value(m_vT.y));
		vT.push_back(value(m_vT.z));
		o.insert(make_pair("vT", value(vT)));

		picojson::array vR;
		vR.push_back(value(m_vR.x));
		vR.push_back(value(m_vR.y));
		vR.push_back(value(m_vR.z));
		o.insert(make_pair("vR", value(vR)));

		string k = picojson::value(o).serialize();

		writeFile(m_paramKiss, k);
	}

}
