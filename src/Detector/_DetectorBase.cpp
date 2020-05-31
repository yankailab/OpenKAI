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

	m_modelFile = "";
	m_trainedFile = "";
	m_meanFile = "";
	m_classFile = "";
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
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//general

	//model
	string modelDir = "";
	F_INFO(pK->v("modelDir", &modelDir));

	pK->v<string>("modelFile", &m_modelFile);
	pK->v<string>("trainedFile", &m_trainedFile);
	pK->v<string>("meanFile", &m_meanFile);
	pK->v<string>("classFile", &m_classFile);

	m_modelFile = modelDir + m_modelFile;
	m_trainedFile = modelDir + m_trainedFile;
	m_meanFile = modelDir + m_meanFile;

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

	string iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pV = (_VisionBase*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_DepthVisionBase",&iName);
	m_pDV = (_DepthVisionBase*)(pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_DetectorBase", &iName);
	m_pDB = (_DetectorBase*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_Universe", &iName);
	m_pU = (_Universe*) (pK->root()->getChildInst(iName));

	return true;
}

int _DetectorBase::check(void)
{
	return 0;
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
	this->_ThreadBase::draw();

	addMsg("nObj=" + i2str(m_pU->size()), 1);

}

}
#endif
