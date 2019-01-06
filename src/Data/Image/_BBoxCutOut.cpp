/*
 * _BBoxCutOut.cpp
 *
 *  Created on: Sept 16, 2017
 *      Author: yankai
 */

#include "../Image/_BBoxCutOut.h"

namespace kai
{

_BBoxCutOut::_BBoxCutOut()
{
	m_extImgIn = ".jpeg";
	m_extImgOut = ".png";
	m_extTxt = ".txt";
}

_BBoxCutOut::~_BBoxCutOut()
{
}

bool _BBoxCutOut::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, dirIn);
	KISSm(pK, dirOut);
	KISSm(pK, extImgIn);
	KISSm(pK, extImgOut);
	KISSm(pK, extTxt);

	return true;
}

bool _BBoxCutOut::start(void)
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

void _BBoxCutOut::update(void)
{
	process();

	exit(0);
}

void _BBoxCutOut::process()
{
	if (m_dirIn.at(m_dirIn.length() - 1) != '/')
		m_dirIn.push_back('/');
	if (m_dirOut.at(m_dirOut.length() - 1) != '/')
		m_dirOut.push_back('/');

	DIR* pDirIn;
	DIR* pDirOut;

	pDirIn = opendir(m_dirIn.c_str());
	pDirOut = opendir(m_dirOut.c_str());

	if (!pDirIn)
	{
		LOG_E("Input directory not found");
		exit(1);
	}

	if (!pDirOut)
	{
		if(mkdir(m_dirOut.c_str(), 0777) != 0)
		{
			LOG_E("Failed to creat output directory");
			exit(1);
		}

		LOG_I("Created output directory: " + m_dirOut);
	}

	vector<int> PNGcompress;
	PNGcompress.push_back(IMWRITE_PNG_COMPRESSION);
	PNGcompress.push_back(0);

	struct dirent *dir;
	string fileItem;
	string fileIn;
	string fileImg;
	string strBB;
	size_t extPos;
	ifstream ifsTxt;
	int nTot = 0;

	while ((dir = readdir(pDirIn)) != NULL)
	{
		fileIn = m_dirIn + dir->d_name;

		//verify txt file
		extPos = fileIn.find(m_extTxt);
		IF_CONT(extPos == std::string::npos);
		IF_CONT(fileIn.substr(extPos) != m_extTxt);
		ifsTxt.open(fileIn.c_str(), std::ios::in);
		IF_CONT(!ifsTxt);

		//skip if no correspondent image file is found
		fileImg = m_dirIn + dir->d_name;
		fileImg.erase(fileImg.find(m_extTxt));
		fileImg += m_extImgIn;
		Mat mImg = cv::imread(fileImg.c_str());
		IF_CONT(mImg.empty());

		//get the name without extension
		fileItem = dir->d_name;
		fileItem.erase(fileItem.find(m_extTxt));

		//read image and cut out bboxes
		int nBB = 0;
		while (getline(ifsTxt, strBB))
		{
			vector<string> entry = splitBy(strBB, ' ');
			IF_CONT(entry.size() < 15);

			string strCat = entry[0];
			Rect cropBB;
			cropBB.x = atoi(entry[4].c_str());
			cropBB.y = atoi(entry[5].c_str());
			cropBB.width = atoi(entry[6].c_str()) - cropBB.x;
			cropBB.height = atoi(entry[7].c_str()) - cropBB.y;

			IF_CONT(cropBB.x < 0);
			IF_CONT(cropBB.y < 0);
			IF_CONT(cropBB.width <= 0);
			IF_CONT(cropBB.height <= 0);
			IF_CONT(cropBB.x + cropBB.width > mImg.cols);
			IF_CONT(cropBB.y + cropBB.height > mImg.rows);

			Mat mBB = Mat(mImg, cropBB);
			IF_CONT(mBB.empty());

			//make directory if not existed
			string dirCat = m_dirOut + strCat + '/';
			if(!opendir(dirCat.c_str()))
			{
				IF_CONT(mkdir(dirCat.c_str(), 0777) != 0);
			}

			string fOut = dirCat + fileItem + "_" + strCat + "_" + i2str(nBB) + m_extImgOut;
			cv::imwrite(fOut,mBB,PNGcompress);

			nBB++;
			nTot++;
			LOG_I(i2str(nTot) + " " + fOut);
		}

		ifsTxt.close();
	}

	LOG_I("Total images cutout: " + i2str(nTot));
	closedir(pDirIn);
	closedir(pDirOut);
}

}

