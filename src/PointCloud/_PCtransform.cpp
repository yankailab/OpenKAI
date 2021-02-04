/*
 * _PCtransform.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCtransform.h"

#ifdef USE_OPEN3D

namespace kai
{

_PCtransform::_PCtransform()
{
	m_vT.init(0);
	m_vR.init(0);
	m_paramKiss = "";
}

_PCtransform::~_PCtransform()
{
}

bool _PCtransform::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vT", &m_vT);
	pK->v("vR", &m_vR);

	int nM;
	pK->v("nMt", &nM);
	for(; nM > 0; nM--)
		m_vmT.push_back(Eigen::Matrix4d::Identity());

	//read from external kiss file if there is one
	pK->v("paramKiss", &m_paramKiss);
	IF_T(m_paramKiss.empty());

	_File* pFile = new _File();
	IF_T(!pFile->open(&m_paramKiss));

	string fn;
    pFile->readAll(&fn);
	IF_T(fn.empty() );

	Kiss* pKf = new Kiss();
	if(pKf->parse( &fn ))
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
	NULL__(m_pPCB, -1);

	return this->_PCbase::check();
}

void _PCtransform::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		updateTransform();
        updatePC();
       
  		if(m_pViewer)
		{
			m_pViewer->updateGeometry(m_iV, m_sPC.prev());
		}

		m_pT->autoFPSto();
	}
}

void _PCtransform::updateTransform(void)
{
	IF_(check()<0);

	Eigen::Matrix4d mT = Eigen::Matrix4d::Identity();
	Eigen::Vector3d vR(m_vR.x, m_vR.y, m_vR.z);
	mT.block(0,0,3,3) = m_sPC.next()->GetRotationMatrixFromXYZ(vR);
	mT(0,3) = m_vT.x;
	mT(1,3) = m_vT.y;
	mT(2,3) = m_vT.z;

	for(Eigen::Matrix4d m : m_vmT)
		mT *= m;

	m_pPCB->getPC(m_sPC.next());
	m_sPC.next()->Transform(mT);
}

void _PCtransform::setTranslation(vFloat3& vT)
{
	m_vT = vT;
}

vFloat3 _PCtransform::getTranslation(void)
{
	return m_vT;
}

void _PCtransform::setRotation(vFloat3& vR)
{
	m_vR = vR;
}

vFloat3 _PCtransform::getRotation(void)
{
	return m_vR;
}

void _PCtransform::setTranslationMatrix(int i, Eigen::Matrix4d_u& mR)
{
	IF_(i >= m_vmT.size());
	m_vmT[i] = mR;
}

Eigen::Matrix4d _PCtransform::getTranslationMatrix(int i)
{
	if(i >= m_vmT.size())
		return Eigen::Matrix4d::Identity();

	return m_vmT[i];
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

	_File* pFile = new _File();
	IF_(!pFile->open(&m_paramKiss, ios::out));
	pFile->write((uint8_t*)k.c_str(), k.length());
	pFile->close();
}

}
#endif
