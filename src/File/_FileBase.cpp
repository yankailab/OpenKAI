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
	}

	_FileBase::~_FileBase()
	{
		m_vExt.clear();
	}

	bool _FileBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
		pK->m_pInst = this;

		m_vExt.clear();
		pK->a("vExt", &m_vExt);

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
		cmd = "mkdir " + dir;
		system(cmd.c_str());

		return true;

		// cmd = "mkdir /media/usb";
		// system(cmd.c_str());
		// cmd = "mount /dev/sda1 /media/usb";
		// system(cmd.c_str());
	}

	bool _FileBase::removeDir(const string &dir)
	{
		string cmd;
		cmd = "rm -r " + dir;
		system(cmd.c_str());

		return true;
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

	bool _FileBase::getDirFileList(const string &dir, vector<string> *pvFile, vector<string> *pvExt)
	{
		NULL_F(pvFile);
		DIR *pDir = opendir(dir.c_str());
		NULL_F(pDir);

		struct dirent *dirE;
		ifstream ifs;
		while ((dirE = readdir(pDir)) != NULL)
		{
			IF_CONT(dirE->d_name[0] == '.');

			string s = dir + dirE->d_name;
			if (dirE->d_type == D_TYPE_FOLDER)
			{
				s = checkDirName(s);
				getDirFileList(s, pvFile, pvExt);
				continue;
			}

			// filter extension
			if (pvExt)
			{
				if (!pvExt->empty())
				{
					IF_CONT(!bExtension(s, *pvExt));
				}
			}

			// verify can be opened
			ifs.open(s.c_str(), std::ios::in);
			IF_CONT(!ifs);
			ifs.close();

			pvFile->push_back(s);
		}

		closedir(pDir);
		return true;
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

	bool _FileBase::bExtension(const string &fName, const vector<string> &vExt)
	{
		string ext = getExtension(fName);
		for (int i = 0; i < vExt.size(); i++)
		{
			IF_T(vExt[i] == ext);
		}

		return false;
	}

}
