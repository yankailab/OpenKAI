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
	m_extIn = ".jpeg";
	m_extOut = ".png";
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
	KISSm(pK, dirOut);
	KISSm(pK, extIn);
	KISSm(pK, extOut);

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

void _DataBase::getInputFileList(void)
{
	m_vFileIn.clear();

	if (m_dirIn.at(m_dirIn.length() - 1) != '/')
		m_dirIn.push_back('/');

	DIR* pDirIn = opendir(m_dirIn.c_str());

	if (!pDirIn)
	{
		LOG_E("Input directory not found");
		exit(1);
	}

	struct dirent *dir;
	string fileIn;
	size_t extPos;
	ifstream ifs;

	while ((dir = readdir(pDirIn)) != NULL)
	{
		fileIn = m_dirIn + dir->d_name;

		//verify file extension
		extPos = fileIn.find(m_extIn);
		IF_CONT(extPos == std::string::npos);
		IF_CONT(fileIn.substr(extPos) != m_extIn);
		ifs.open(fileIn.c_str(), std::ios::in);
		IF_CONT(!ifs);
		ifs.close();

		m_vFileIn.push_back(fileIn);
	}

	closedir(pDirIn);
}

void _DataBase::openOutput(void)
{
	if (m_dirOut.at(m_dirOut.length() - 1) != '/')
		m_dirOut.push_back('/');

	DIR* pDirOut = opendir(m_dirOut.c_str());

	if (!pDirOut)
	{
		if(mkdir(m_dirOut.c_str(), 0777) != 0)
		{
			LOG_E("Failed to creat output directory");
			exit(1);
		}

		LOG_I("Created output directory: " << m_dirOut);
	}







	closedir(pDirOut);
}

}

