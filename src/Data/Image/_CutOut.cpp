/*
 * _CutOut.cpp
 *
 *  Created on: Sept 16, 2017
 *      Author: yankai
 */

#include "../Image/_CutOut.h"

namespace kai
{

_CutOut::_CutOut()
{
	m_extImgIn = ".png";
	m_extImgOut = ".png";
	m_nMinPixel = 10;
}

_CutOut::~_CutOut()
{
}

bool _CutOut::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, extImgIn);
	KISSm(pK, extImgOut);
	KISSm(pK, extImgOut);
	KISSm(pK, nMinPixel);

	Kiss* pDir = pK->o("dir");
	IF_T(pDir->empty());
	Kiss** pItr = pDir->getChildItr();

	CutOutDir D;
	unsigned int i = 0;
	while (pItr[i])
	{
		Kiss* pCD = pItr[i++];

		D.init();
		pCD->v("dirRGBIn",&D.m_dirRGBIn);
		pCD->v("dirRGBOut",&D.m_dirRGBOut);
		pCD->v("dirSegIn",&D.m_dirSegIn);
		pCD->v("dirSegOut",&D.m_dirSegOut);
		pCD->v("w",&D.m_w);
		pCD->v("h",&D.m_h);
		pCD->v("dW",&D.m_dW);
		pCD->v("dH",&D.m_dH);
		D.setup();

		m_vDir.push_back(D);
	}

	return true;
}

bool _CutOut::start(void)
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

void _CutOut::update(void)
{
	for(int i=0; i<m_vDir.size(); i++)
	{
		process(&m_vDir[i]);
	}

	LOG_I("------------------Complete");
}

void _CutOut::process(CutOutDir* pD)
{
	NULL_(pD);

	pD->m_dirRGBIn = checkDirName(pD->m_dirRGBIn);
	pD->m_dirRGBOut = checkDirName(pD->m_dirRGBOut);
	pD->m_dirSegIn = checkDirName(pD->m_dirSegIn);
	pD->m_dirSegOut = checkDirName(pD->m_dirSegOut);

	//open input directory in get file list
	DIR* pDirRGBIn = opendir(pD->m_dirRGBIn.c_str());
	NULL_l(pDirRGBIn, "RGB Input directory not found: " + pD->m_dirRGBIn);

	struct dirent *dirRGB;
	string fRGBIn;
	m_vRGBIn.clear();
	while ((dirRGB = readdir(pDirRGBIn)) != NULL)
	{
		fRGBIn = pD->m_dirRGBIn + dirRGB->d_name;
		IF_CONT(getFileExt(fRGBIn) != m_extImgIn);
		m_vRGBIn.push_back(fRGBIn);
	}

	closedir(pDirRGBIn);

	DIR* pDirSegIn = opendir(pD->m_dirSegIn.c_str());
	NULL_l(pDirSegIn, "Seg Input directory not found: " + pD->m_dirSegIn);

	struct dirent *dirSeg;
	string fSegIn;
	m_vSegIn.clear();
	while ((dirSeg = readdir(pDirSegIn)) != NULL)
	{
		fSegIn = pD->m_dirSegIn + dirSeg->d_name;
		IF_CONT(getFileExt(fSegIn) != m_extImgIn);
		m_vSegIn.push_back(fSegIn);
	}

	closedir(pDirSegIn);

	//prepare output directory
	DIR* pDirRGBOut = opendir(pD->m_dirRGBOut.c_str());
	if (!pDirRGBOut)
	{
		IF_l(mkdir(pD->m_dirRGBOut.c_str(), 0777) != 0,
			 "Failed to creat RGB output directory: " + pD->m_dirRGBOut);
		LOG_I("Created RGB output directory: " + pD->m_dirRGBOut);
	}

	DIR* pDirSegOut = opendir(pD->m_dirSegOut.c_str());
	if (!pDirSegOut)
	{
		IF_l(mkdir(pD->m_dirSegOut.c_str(), 0777) != 0,
			 "Failed to creat Seg output directory: " + pD->m_dirSegOut);
		LOG_I("Created Seg output directory: " + pD->m_dirSegOut);
	}

	//constants
	vector<int> PNGcompress;
	PNGcompress.push_back(IMWRITE_PNG_COMPRESSION);
	PNGcompress.push_back(1);
	int iTotRGB = 1;
	int iTotSeg = 0;

	//process all
	for(int i=0; i<m_vRGBIn.size(); i++)
	{
		fRGBIn = m_vRGBIn[i];
		int iRGB = getFileIdx(fRGBIn);
		Mat mRGB = cv::imread(fRGBIn.c_str());
		string fRGB = getFileWithOutIdx(fRGBIn);
		Size s = Size(mRGB.cols, mRGB.rows);

		for(int m=0; m<pD->m_vBB.size(); m++)
		{
			vDouble4 fBB = pD->m_vBB[m];
			vInt4 iBB;
			iBB.x = fBB.x * mRGB.cols;
			iBB.y = fBB.y * mRGB.rows;
			iBB.z = fBB.z * mRGB.cols;
			iBB.w = fBB.w * mRGB.rows;

			Rect r;
			vInt42rect(iBB,r);

			Mat mBB = Mat(mRGB, r);
			IF_CONT(mBB.empty());
			Mat mBBs;
			cv::resize(mBB, mBBs, s, INTER_LINEAR);
			mBB = mBBs;

			char buf[UTIL_BUF];
			snprintf(buf, UTIL_BUF, "%03d", iTotRGB+m);
			string fOut = pD->m_dirRGBOut + fRGB + "_" + buf + m_extImgOut;
			cv::imwrite(fOut,mBB,PNGcompress);
		}

		for(int j=0; j<m_vSegIn.size(); j++)
		{
			fSegIn = m_vSegIn[j];
			IF_CONT(getFileIdx(fSegIn) != iRGB);

			Mat mSeg = cv::imread(fSegIn.c_str());
			string fSeg = getFileWithOutIdx(fSegIn);

			for(int n=0; n<pD->m_vBB.size(); n++)
			{
				vDouble4 fBB = pD->m_vBB[n];
				vInt4 iBB;
				iBB.x = fBB.x * mSeg.cols;
				iBB.y = fBB.y * mSeg.rows;
				iBB.z = fBB.z * mSeg.cols;
				iBB.w = fBB.w * mSeg.rows;

				Rect r;
				vInt42rect(iBB,r);

				Mat mBB = Mat(mSeg, r);
				IF_CONT(mBB.empty());

				vector<Mat> vSeg(3);
				split(mBB, vSeg);
				IF_CONT(cv::countNonZero(vSeg[0]) < m_nMinPixel);

				Mat mBBs;
				cv::resize(mBB, mBBs, s, INTER_LINEAR);
				mBB = mBBs;

				char buf[UTIL_BUF];
				snprintf(buf, UTIL_BUF, "%03d", iTotRGB+n);
				string fOut = pD->m_dirSegOut + fSeg + "_" + buf + m_extImgOut;
				cv::imwrite(fOut,mBB,PNGcompress);
				iTotSeg++;
			}
		}

		iTotRGB += pD->m_vBB.size();
	}

	LOG_I(pD->m_dirRGBIn + ": iTotRGB: " + i2str(iTotRGB-1));
	LOG_I(pD->m_dirSegIn + ": iTotSeg: " + i2str(iTotSeg));
}

int _CutOut::getFileIdx(string& file)
{
	size_t pExt = file.find_last_of(".");
	if(pExt == std::string::npos)return -1;

	size_t pU = file.find_last_of("_");
	if(pU == std::string::npos)return -1;
	pU++;

	string iStr = file.substr(pU, pExt-pU);
	return stoi(iStr);
}

string _CutOut::getFileWithOutIdx(string& file)
{
	size_t pU = file.find_last_of("_");
	if(pU == std::string::npos)return "";

	size_t pDir = file.find_last_of("/");
	if(pDir == std::string::npos)return "";
	pDir++;

	return file.substr(pDir, pU-pDir);
}

}
