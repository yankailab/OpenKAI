/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_DetectorBase.h"

namespace kai
{

	_DetectorBase::_DetectorBase()
	{
		m_pV = NULL;
		m_pU = NULL;

		m_confidence = 0.5;
		m_fModel = "";
		m_fWeight = "";
		m_fMean = "";
		m_fClass = "";
		m_nClass = 0;
	}

	_DetectorBase::~_DetectorBase()
	{
	}

	bool _DetectorBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		// model
		pK->v("fModel", &m_fModel);
		pK->v("fWeight", &m_fWeight);
		pK->v("fMean", &m_fMean);
		pK->v("fClass", &m_fClass);
		pK->v("confidence", &m_confidence);

		// statistics
		if (!m_fClass.empty())
		{
			ifstream ifs(m_fClass.c_str());
			IF_Fl(!ifs.is_open(), "File " + m_fClass + " not found");

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

			for (int i = 0; i < m_nClass; i++)
			{
				OBJ_CLASS oc;
				oc.init();
				oc.m_name = vClassList[i];
				m_vClass.push_back(oc);
			}
		}

		return true;
	}

	bool _DetectorBase::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n = "";
		F_INFO(pK->v("_VisionBase", &n));
		m_pV = (_VisionBase *)(pK->getInst(n));

		n = "";
		pK->v("_Universe", &n);
		m_pU = (_Universe *)(pK->getInst(n));

		return true;
	}

	int _DetectorBase::check(void)
	{
		return this->_ModuleBase::check();
	}

	int _DetectorBase::getClassIdx(string &className)
	{
		for (int i = 0; i < m_nClass; i++)
		{
			if (m_vClass[i].m_name == className)
				return i;
		}

		return -1;
	}

	string _DetectorBase::getClassName(int iClass)
	{
		if (iClass < 0)
			return "";
		if (iClass >= m_nClass)
			return "";

		return m_vClass[iClass].m_name;
	}

	_Universe* _DetectorBase::getU(void)
	{
		return m_pU;
	}


}
