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
	m_extOut = ".png";
	m_PNGcompression = 0;
}

_DataBase::~_DataBase()
{
	m_PNGcompress.clear();
	m_vExtIn.clear();
}

bool _DataBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, dirIn);
	KISSm(pK, extOut);

	m_vExtIn.clear();
	string pExtIn[N_EXT];
	int nExt = pK->array("extIn", pExtIn, N_EXT);
	for(int i=0; i<nExt; i++)
	{
		m_vExtIn.push_back(pExtIn[i]);
	}

	KISSm(pK, PNGcompression);
	m_PNGcompress.push_back(IMWRITE_PNG_COMPRESSION);
	m_PNGcompress.push_back(m_PNGcompression);

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

void _DataBase::getDirFileList(string* pStrDir)
{
	NULL_(pStrDir);

	DIR* pDirIn = opendir(pStrDir->c_str());
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

		if(dir->d_type == D_TYPE_FOLDER)
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

string _DataBase::getExtension(string& fName)
{
	size_t extPos = fName.find_last_of('.');
	if(extPos == std::string::npos)
		return "";

	static string ext = fName.substr(extPos);
	transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	return ext;
}

bool _DataBase::verifyExtension(string& fName)
{
	for(int i=0; i<m_vExtIn.size(); i++)
	{
		IF_T(m_vExtIn[i] == getExtension(fName));
	}

	return false;
}

}

