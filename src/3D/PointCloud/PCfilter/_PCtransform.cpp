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
		m_jsonCfgFile = "";
	}

	_PCtransform::~_PCtransform()
	{
	}

	bool _PCtransform::init(const json &j)
	{
		IF_F(!this->_PCframe::init(j));

		// read from external json config file if there is one
		m_jsonCfgFile = j.value("jsonCfgFile", "");
		IF__(m_jsonCfgFile.empty(), true);

		JsonCfg jCfg;
		IF__(!jCfg.parseJsonFile(m_jsonCfgFile), true);

		const json &jt = jCfg.getJson().at("transform");
		IF__(!jt.is_object(), true);

		m_vT = jt.value("vT", vector<double>{});
		m_vR = jt.value("vR", vector<double>{});

		return true;
	}

	bool _PCtransform::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _PCtransform::check(void)
	{
		//		NULL_F(m_pInCtx.m_pPCB);

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
		IF_(!check());

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
		IF_(m_jsonCfgFile.empty());

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

		writeFile(m_jsonCfgFile, k);
	}

}
