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
	m_dirOut = "";
	m_extOut = ".png";

	m_PNGcompress.push_back(CV_IMWRITE_PNG_COMPRESSION);
	m_PNGcompress.push_back(0);

}

_DataBase::~_DataBase()
{
}

bool _DataBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, dirIn);
	m_dirOut = m_dirIn;
	KISSm(pK, dirOut);
	KISSm(pK, extOut);

	m_vExtIn.clear();
	string pExtIn[N_EXT];
	int nExt = pK->array("extIn", pExtIn, N_EXT);
	for(int i=0; i<nExt; i++)
	{
		m_vExtIn.push_back(pExtIn[i]);
	}

	return true;
}

bool _DataBase::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _DataBase::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _DataBase::update(void)
{
}

int _DataBase::getDirFileList(void)
{
	m_vFileIn.clear();

	if (m_dirIn.at(m_dirIn.length() - 1) != '/')
		m_dirIn.push_back('/');

	DIR* pDirIn = opendir(m_dirIn.c_str());

	if (!pDirIn)
	{
		LOG_E("Input directory not found");
		return -1;
	}

	struct dirent *dir;
	ifstream ifs;

	while ((dir = readdir(pDirIn)) != NULL)
	{
		string fileIn = m_dirIn + dir->d_name;

		IF_CONT(!verifyExtension(&fileIn));
		ifs.open(fileIn.c_str(), std::ios::in);
		IF_CONT(!ifs);
		ifs.close();

		m_vFileIn.push_back(dir->d_name);
	}

	closedir(pDirIn);

	return m_vFileIn.size();
}

bool _DataBase::verifyExtension(string* fName)
{
	NULL_F(fName);

	int i;
	for(i=0; i<m_vExtIn.size(); i++)
	{
		string ext = m_vExtIn[i];
		size_t extPos = fName->find(ext);
		IF_CONT(extPos == std::string::npos);
		IF_CONT(fName->substr(extPos) != ext);

		return true;
	}

	return false;
}

bool _DataBase::openOutput(void)
{
	if (m_dirOut.at(m_dirOut.length() - 1) != '/')
		m_dirOut.push_back('/');

	DIR* pDirOut = opendir(m_dirOut.c_str());

	if (!pDirOut)
	{
		if(mkdir(m_dirOut.c_str(), 0777) != 0)
		{
			LOG_E("Failed to creat output directory");
			return false;
		}

		LOG_I("Created output directory: " << m_dirOut);
	}

	closedir(pDirOut);
	return true;
}

}

