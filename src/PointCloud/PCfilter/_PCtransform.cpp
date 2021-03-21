/*
 * _PCtransform.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
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

	bool _PCtransform::init(void *pKiss)
	{
		IF_F(!_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		//read from external kiss file if there is one
		pK->v("paramKiss", &m_paramKiss);
		IF_T(m_paramKiss.empty());

		_File *pFile = new _File();
		IF_T(!pFile->open(&m_paramKiss));

		string fn;
		pFile->readAll(&fn);
		IF_T(fn.empty());

		Kiss *pKf = new Kiss();
		if (pKf->parse(&fn))
		{
			pK = pKf->child("transform");
			pK->v("vT", &m_vT);
			pK->v("vR", &m_vR);
		}

		delete pKf;
		pFile->close();
		return true;
	}

	bool _PCtransform::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _PCtransform::check(void)
	{
		NULL__(m_pInCtx.m_pPCB, -1);

		return this->_PCframe::check();
	}

	void _PCtransform::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateTransform();

			m_pT->autoFPSto();
		}
	}

	void _PCtransform::updateTransform(void)
	{
		IF_(check() < 0);

		readPC(m_pInCtx.m_pPCB);
		m_sPC.next()->Transform(m_mTt);
		updatePC();
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

		_File *pFile = new _File();
		IF_(!pFile->open(&m_paramKiss, ios::out));
		pFile->write((uint8_t *)k.c_str(), k.length());
		pFile->close();
	}

}
#endif