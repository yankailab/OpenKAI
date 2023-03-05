/*
 * _FileBase.cpp
 *
 *  Created on: Oct 16, 2017
 *      Author: yankai
 */

#include "_FileBase.h"

namespace kai
{

	_FileBase::_FileBase()
	{
		m_dirIn = "";
	}

	_FileBase::~_FileBase()
	{
		m_vExtIn.clear();
	}

	bool _FileBase::init(void *pKiss)
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

	bool _FileBase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _FileBase::update(void)
	{
	}

	bool _FileBase::createDir(const string &dir)
	{
		string cmd;
		// cmd = "mkdir /media/usb";
		// system(cmd.c_str());
		// cmd = "mount /dev/sda1 /media/usb";
		// system(cmd.c_str());
		cmd = "mkdir " + dir;
		system(cmd.c_str());
	}

	string _FileBase::getFirstSubDir(const string &baseDir)
	{
		DIR *pDir = opendir(baseDir.c_str());
		if (!pDir)
			return "";

		struct dirent *dir;
		string d = "";
		while ((dir = readdir(pDir)) != NULL)
		{
			IF_CONT(dir->d_name[0] == '.');
			IF_CONT(dir->d_type != D_TYPE_FOLDER);

			d = baseDir + string(dir->d_name);
			d = checkDirName(d);
			break;
		}

		closedir(pDir);
		return d;
	}

	void _FileBase::getDirFileList(const string &dir)
	{
		DIR *pDirIn = opendir(dir.c_str());
		if (!pDirIn)
		{
			LOG_E("Directory not found: " + dir);
			return;
		}

		struct dirent *dirE;
		ifstream ifs;

		while ((dirE = readdir(pDirIn)) != NULL)
		{
			IF_CONT(dirE->d_name[0] == '.');

			string dirIn = dir + dirE->d_name;

			if (dirE->d_type == D_TYPE_FOLDER)
			{
				dirIn = checkDirName(dirIn);
				getDirFileList(dirIn);
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

	string _FileBase::getExtension(const string &fName)
	{
		size_t extPos = fName.find_last_of('.');
		if (extPos == std::string::npos)
			return "";

		string ext = fName.substr(extPos);
		transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		return ext;
	}


	void _FileBase::setFileList(vector<string> vFileIn)
	{
		m_vFileIn.clear();
		m_vFileIn = vFileIn;
	}

	// int _FileBase::getDirFileList(void)
	// {
	// 	m_vFileIn.clear();

	// 	m_dirIn = checkDirName(m_dirIn);
	// 	getDirFileList(&m_dirIn);

	// 	return m_vFileIn.size();
	// }


	bool _FileBase::verifyExtension(const string &fName)
	{
		for (int i = 0; i < m_vExtIn.size(); i++)
		{
			IF_T(m_vExtIn[i] == getExtension(fName));
		}

		return false;
	}

}
