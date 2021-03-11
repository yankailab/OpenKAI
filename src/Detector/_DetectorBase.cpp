/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_DetectorBase.h"

#ifdef USE_OPENCV

namespace kai
{

_DetectorBase::_DetectorBase()
{
	m_pV = NULL;
	m_pDV = NULL;
	m_pDB = NULL;
	m_pU = NULL;

	m_fModel = "";
	m_fWeight = "";
	m_fMean = "";
	m_fClass = "";
	m_nClass = 0;

	m_bDrawStatistics = false;
	m_classLegendPos.x = 25;
	m_classLegendPos.y = 100;
	m_classLegendPos.z = 15;
	m_bDrawClass = false;
	m_bDrawText = false;
	m_bDrawPos = false;
}

_DetectorBase::~_DetectorBase()
{
}

bool _DetectorBase::init(void* pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//general

	//model
	pK->v("fModel", &m_fModel);
	pK->v("fWeight", &m_fWeight);
	pK->v("fMean", &m_fMean);
	pK->v("fClass", &m_fClass);

	//statistics
	string s;
	pK->v("classFile", &s);
	if(!s.empty())
	{
		ifstream ifs(s.c_str());
	    IF_Fl(!ifs.is_open(),"File " + s + " not found");

	    string line;
		while (std::getline(ifs, line))
		{
			OBJ_CLASS oc;
			oc.init();
			oc.m_name = line;
			m_vClass.push_back(oc);
		}

		m_nClass = m_vClass.size();
		ifs.close();
	}
	else
	{
		vector<string> vClassList;
		m_nClass = pK->a("classList", &vClassList);

		for(int i=0; i<m_nClass; i++)
		{
			OBJ_CLASS oc;
			oc.init();
			oc.m_name = vClassList[i];
			m_vClass.push_back(oc);
		}
	}

	//draw
	pK->v<bool>("bDrawStatistics", &m_bDrawStatistics);
	pK->v<bool>("bDrawClass", &m_bDrawClass);
	pK->v<bool>("bDrawText", &m_bDrawText);
	pK->v<bool>("bDrawPos", &m_bDrawPos);

	string n = "";
	F_INFO(pK->v("_VisionBase", &n));
	m_pV = (_VisionBase*) (pK->getInst(n));

	n = "";
	pK->v("_DepthVisionBase",&n);
	m_pDV = (_DepthVisionBase*)(pK->getInst(n));

	n = "";
	pK->v("_DetectorBase", &n);
	m_pDB = (_DetectorBase*) (pK->getInst(n));

	n = "";
	pK->v("_Universe", &n);
	m_pU = (_Universe*) (pK->getInst(n));

	return true;
}

int _DetectorBase::check(void)
{
	return this->_ModuleBase::check();
}

int _DetectorBase::getClassIdx(string& className)
{
	for(int i=0; i<m_nClass; i++)
	{
		if(m_vClass[i].m_name == className)return i;
	}

	return -1;
}

string _DetectorBase::getClassName(int iClass)
{
	if(iClass < 0)return "";
	if(iClass >= m_nClass)return "";

	return m_vClass[iClass].m_name;
}

void _DetectorBase::draw(void)
{
	this->_ModuleBase::draw();

}

}
#endif
