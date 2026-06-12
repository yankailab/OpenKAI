/*
 * _Img2Frame.cpp
 *
 *  Created on: June 12, 2026
 *      Author: yankai
 */

#include "_Img2Frame.h"

namespace kai
{
	static bool img2FrameIsPng(const string &file)
	{
		size_t extPos = file.find_last_of(".");
		IF_F(extPos == string::npos);

		string ext = file.substr(extPos);
		return (ext == ".png") || (ext == ".PNG");
	}

	static _Img2Frame::ImgFile img2FrameParseFile(const string &dir, const string &file)
	{
		_Img2Frame::ImgFile imgFile;
		imgFile.file = dir + file;
		imgFile.time = file;
		imgFile.index = 0;
		imgFile.bParsed = false;

		size_t extPos = file.find_last_of(".");
		size_t idxPos = file.find_last_of("_", extPos);
		IF__(extPos == string::npos, imgFile);
		IF__(idxPos == string::npos, imgFile);
		IF__(idxPos + 1 >= extPos, imgFile);

		string indexStr = file.substr(idxPos + 1, extPos - idxPos - 1);
		char *pEnd = nullptr;
		long index = strtol(indexStr.c_str(), &pEnd, 10);
		IF__(pEnd == indexStr.c_str(), imgFile);
		IF__(*pEnd != '\0', imgFile);

		imgFile.time = file.substr(0, idxPos);
		imgFile.index = (int)index;
		imgFile.bParsed = true;

		return imgFile;
	}

	static bool img2FrameFileLess(const _Img2Frame::ImgFile &a, const _Img2Frame::ImgFile &b)
	{
		if (a.bParsed != b.bParsed)
			return a.bParsed > b.bParsed;

		if (a.time != b.time)
			return a.time < b.time;

		if (a.index != b.index)
			return a.index < b.index;

		return a.file < b.file;
	}

	_Img2Frame::_Img2Frame()
	{
		m_type = vision_Img2Frame;

		m_dir = "";
		m_iFrame = 0;
		m_vOffset = -50.0;
		m_vScale = 64.0;
		m_bLoop = true;
	}

	_Img2Frame::~_Img2Frame()
	{
		close();
	}

	bool _Img2Frame::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jKv(j, "dir", m_dir);
		jKv(j, "vOffset", m_vOffset);
		jKv(j, "vScale", m_vScale);
		jKv(j, "bLoop", m_bLoop);

		IF_F(m_dir.empty());
		m_dir = checkDirName(m_dir);
		m_iFrame = 0;

		return true;
	}

	bool _Img2Frame::open(void)
	{
		IF_F(!check());
		IF__(m_bOpen, true);
		IF_F(!loadFrameBuffer());

		m_bOpen = true;
		return true;
	}

	void _Img2Frame::close(void)
	{
		this->_VisionBase::close();

		m_frameBuffer.clear();
		m_iFrame = 0;
	}

	bool _Img2Frame::loadFrameBuffer(void)
	{
		m_frameBuffer.clear();
		m_iFrame = 0;

		vector<ImgFile> imgFiles = getImgFiles();
		IF_F(imgFiles.empty());
		IF_F(m_vScale == 0.0);

		for (int i = 0; i < (int)imgFiles.size(); i++)
		{
			Mat mIn = imread(imgFiles[i].file, IMREAD_UNCHANGED);
			if (mIn.empty())
			{
				LOG_E("Cannot open file: " + imgFiles[i].file);
				continue;
			}

			IF_CONT(mIn.channels() != 1);

			Mat mFrame;
			mIn.convertTo(mFrame, CV_32FC1, 1.0 / m_vScale, m_vOffset);
			m_frameBuffer.push_back(mFrame);
		}

		IF_F(m_frameBuffer.empty());

		m_vSizeRGB.x = m_frameBuffer[0].cols;
		m_vSizeRGB.y = m_frameBuffer[0].rows;
		m_fRGB.copy(m_frameBuffer[0]);

		return true;
	}

	vector<_Img2Frame::ImgFile> _Img2Frame::getImgFiles(void)
	{
		vector<ImgFile> imgFiles;

		DIR *pDir = opendir(m_dir.c_str());
		if (!pDir)
		{
			LOG_E("Cannot open dir: " + m_dir);
			return imgFiles;
		}

		struct dirent *pEnt = NULL;
		while ((pEnt = readdir(pDir)) != NULL)
		{
			string file = pEnt->d_name;
			IF_CONT(file == ".");
			IF_CONT(file == "..");
			IF_CONT(!img2FrameIsPng(file));

			imgFiles.push_back(img2FrameParseFile(m_dir, file));
		}

		closedir(pDir);
		sort(imgFiles.begin(), imgFiles.end(), img2FrameFileLess);

		return imgFiles;
	}

	bool _Img2Frame::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Img2Frame::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					close();
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();

			updateFrame();
		}
	}

	void _Img2Frame::updateFrame(void)
	{
		IF_(m_frameBuffer.empty());

		if (m_iFrame >= (int)m_frameBuffer.size())
		{
			IF_(!m_bLoop);
			m_iFrame = 0;
		}

		m_fRGB.copy(m_frameBuffer[m_iFrame]);
		m_iFrame++;
	}

	void _Img2Frame::console(const json &j, void *pJSONbase)
	{
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		if (cmd == "reload")
		{
			close();
			open();
		}

		this->_VisionBase::console(j, pJSONbase);
	}

}
