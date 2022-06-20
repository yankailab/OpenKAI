/*
 * _DataBase.cpp
 *
 *  Created on: Oct 16, 2017
 *      Author: yankai
 */

#include "_DataBase.h"

namespace kai
{

	_DataBase::_DataBase()
	{
		m_dirIn = "";
	}

	_DataBase::~_DataBase()
	{
		m_vExtIn.clear();
	}

	bool _DataBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("dirIn", &m_dirIn);

		m_vExtIn.clear();
		vector<string> vExtIn;
		pK->a("extIn", &vExtIn);
		for (int i = 0; i < vExtIn.size(); i++)
		{
			m_vExtIn.push_back(vExtIn[i]);
		}

		return true;
	}

	bool _DataBase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _DataBase::update(void)
	{
	}

	void _DataBase::setFileList(vector<string> vFileIn)
	{
		m_vFileIn.clear();
		m_vFileIn = vFileIn;
	}

	int _DataBase::getDirFileList(void)
	{
		m_vFileIn.clear();

		m_dirIn = checkDirName(m_dirIn);
		getDirFileList(&m_dirIn);

		return m_vFileIn.size();
	}

	void _DataBase::getDirFileList(string *pStrDir)
	{
		NULL_(pStrDir);

		DIR *pDirIn = opendir(pStrDir->c_str());
		if (!pDirIn)
		{
			LOG_E("Directory not found: " + *pStrDir);
			return;
		}

		struct dirent *dir;
		ifstream ifs;

		while ((dir = readdir(pDirIn)) != NULL)
		{
			IF_CONT(dir->d_name[0] == '.');

			string dirIn = *pStrDir + dir->d_name;

			if (dir->d_type == D_TYPE_FOLDER)
			{
				dirIn = checkDirName(dirIn);
				getDirFileList(&dirIn);
				continue;
			}

			IF_CONT(!verifyExtension(dirIn));
			ifs.open(dirIn.c_str(), std::ios::in);
			IF_CONT(!ifs);
			ifs.close();

			m_vFileIn.push_back(dirIn);
		}

		closedir(pDirIn);
	}

	string _DataBase::getExtension(string &fName)
	{
		size_t extPos = fName.find_last_of('.');
		if (extPos == std::string::npos)
			return "";

		string ext = fName.substr(extPos);
		transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		return ext;
	}

	bool _DataBase::verifyExtension(string &fName)
	{
		for (int i = 0; i < m_vExtIn.size(); i++)
		{
			IF_T(m_vExtIn[i] == getExtension(fName));
		}

		return false;
	}

}
