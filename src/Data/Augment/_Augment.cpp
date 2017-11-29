/*
 * _Augment.cpp
 *
 *  Created on: Oct 12, 2017
 *      Author: yankai
 */

#include "_Augment.h"

namespace kai
{

_Augment::_Augment()
{
	m_bgNoiseMean = 0;
	m_bgNoiseDev = 0;
	m_bgNoiseType = cv::RNG::NORMAL;

	m_nRot = 1;

	m_dCrop = 0.25;
	m_nCrop = 1;

	m_dShrink = 0.25;
	m_nShrink = 1;

	m_minLowResolution = 0.25;
	m_dLowResolution = 0.5;
	m_nLowResolution = 1;

	m_nNoise = 1;
	m_dNoise = 32;
	m_noiseType = cv::RNG::NORMAL;

	m_dContrast = 0.5;
	m_nContrast = 1;

	m_dBrightness = 10;
	m_nBrightness = 1;

	m_dBlur = 10;
	m_nBlur = 1;

	m_dGaussianBlur = 10;
	m_nGaussianBlur = 1;

	m_dMedianBlur = 10;
	m_nMedianBlur = 1;

	m_dBilateralBlur = 10;
	m_nBilateralBlur = 1;

	m_bDeleteOriginal = false;
	m_bSaveOriginalCopy = false;
	m_progress = 0.0;
	m_pFrameIn = NULL;
	m_pFrameOut = NULL;
}

_Augment::~_Augment()
{
}

bool _Augment::init(void* pKiss)
{
	IF_F(!this->_DataBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, bgNoiseMean);
	KISSm(pK, bgNoiseDev);
	KISSm(pK, bgNoiseType);

	KISSm(pK, nRot);

	KISSm(pK, dCrop);
	KISSm(pK, nCrop);

	KISSm(pK, dShrink);
	KISSm(pK, nShrink);

	KISSm(pK, minLowResolution);
	KISSm(pK, dLowResolution);
	KISSm(pK, nLowResolution);

	KISSm(pK, nNoise);
	KISSm(pK, dNoise);
	KISSm(pK, noiseType);

	KISSm(pK, dContrast);
	KISSm(pK, nContrast);

	KISSm(pK, dBrightness);
	KISSm(pK, nBrightness);

	KISSm(pK, bDeleteOriginal);
	KISSm(pK, bSaveOriginalCopy);

	KISSm(pK, dBlur);
	KISSm(pK, nBlur);

	KISSm(pK, dGaussianBlur);
	KISSm(pK, nGaussianBlur);

	KISSm(pK, dMedianBlur);
	KISSm(pK, nMedianBlur);

	KISSm(pK, dBilateralBlur);
	KISSm(pK, nBilateralBlur);

	m_vCmd.clear();
	string pCmdIn[N_CMD];
	int nCmd = pK->array("cmd", pCmdIn, N_CMD);
	for (int i = 0; i < nCmd; i++)
	{
		m_vCmd.push_back(pCmdIn[i]);
	}

	m_pFrameIn = new Frame();
	m_pFrameOut = new Frame();

	return true;
}

bool _Augment::link(void)
{
	IF_F(!this->_DataBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _Augment::start(void)
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

void _Augment::update(void)
{
	srand(time(NULL));

	for (int i = 0; i < m_vCmd.size(); i++)
	{
		string cmd = m_vCmd[i];

		if (cmd == "rotate")
			rotate();
		else if (cmd == "shrink")
			shrink();
		else if (cmd == "crop")
			crop();
		else if (cmd == "flip")
			flip();
		else if (cmd == "lowResolution")
			lowResolution();
		else if (cmd == "noise")
			noise();
		else if (cmd == "contrast")
			contrast();
		else if (cmd == "brightness")
			brightness();
		else if (cmd == "histEqualize")
			histEqualize();
		else if (cmd == "blur")
			blur();
		else if (cmd == "gaussianBlur")
			gaussianBlur();
		else if (cmd == "medianBlur")
			medianBlur();
		else if (cmd == "bilateralBlur")
			bilateralBlur();
		else
		{
			LOG_E("Unrecognized augment cmd: "<<cmd);
		}
	}

	LOG_I("------------------------------Completed");
}

void _Augment::rotate(void)
{
	IF_(getDirFileList() <= 0);
	cv::RNG gen(cv::getTickCount());

	int nTot = 0;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		Point2f pCenter = Point2f(mIn.cols / 2, mIn.rows / 2);

		for (int j = 0; j < m_nRot; j++)
		{
			Size s = mIn.size();
			Mat mMask = Mat::zeros(s, mIn.type());
			Mat mNoise = Mat::zeros(s, mIn.type());
			gen.fill(mNoise, m_bgNoiseType,
					cv::Scalar(m_bgNoiseMean, m_bgNoiseMean, m_bgNoiseMean),
					cv::Scalar(m_bgNoiseDev, m_bgNoiseDev, m_bgNoiseDev));

			int rAngle = rand() % 358 + 1;
			Mat mRot = getRotationMatrix2D(pCenter, rAngle, 1);

			Mat mOut = Mat::zeros(s, mIn.type());
			cv::warpAffine(Mat(s, mIn.type(), Scalar(255, 255, 255)), mMask,
					mRot, s, INTER_LINEAR, BORDER_CONSTANT);
			cv::warpAffine(mIn, mOut, mRot, s, INTER_LINEAR, BORDER_CONSTANT);

			mOut.copyTo(mNoise, mMask);
			mOut = mNoise;

			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Rot: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total rotated: " << nTot);
}

void _Augment::noise(void)
{
	IF_(getDirFileList() <= 0);
	cv::RNG gen(cv::getTickCount());

	int nTot = 0;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		for (int j = 0; j < m_nNoise; j++)
		{
			Mat mNoise = Mat::zeros(mIn.size(), mIn.type());
			gen.fill(mNoise, m_noiseType,
					cv::Scalar(m_dNoise, m_dNoise, m_dNoise),
					cv::Scalar(m_dNoise, m_dNoise, m_dNoise));

			Mat mOut = mIn + mNoise - cv::Scalar(m_dNoise);
			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Noise: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total noise added: " << nTot);
}

void _Augment::shrink(void)
{
	IF_(getDirFileList() <= 0);
	cv::RNG gen(cv::getTickCount());

	int nTot = 0;
	Mat mOut;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		for (int j = 0; j < m_nShrink; j++)
		{
			Size s = mIn.size();
			Mat mMask = Mat::zeros(s, mIn.type());
			Mat mNoise = Mat::zeros(s, mIn.type());
			gen.fill(mNoise, m_bgNoiseType,
					cv::Scalar(m_bgNoiseMean, m_bgNoiseMean, m_bgNoiseMean),
					cv::Scalar(m_bgNoiseDev, m_bgNoiseDev, m_bgNoiseDev));

			m_pFrameIn->update(&mIn);
			m_pFrameOut->getResizedOf(m_pFrameIn, 1.0 - m_dShrink * NormRand(),
					1.0 - m_dShrink * NormRand());

			Mat sMat = *m_pFrameOut->getCMat();
			mOut = mNoise;
			sMat.copyTo(
					mOut(
							cv::Rect((int) ((double) (mOut.cols - sMat.cols) * NormRand()),
									 (int) ((double) (mOut.rows - sMat.rows) * NormRand()),
									sMat.cols,
									sMat.rows)));

			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Shrink: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total shrinked: " << nTot);
}

void _Augment::lowResolution(void)
{
	IF_(getDirFileList() <= 0);

	int nTot = 0;
	Mat mOut;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		for (int j = 0; j < m_nShrink; j++)
		{
			double scale = m_minLowResolution + m_dLowResolution * NormRand();
			m_pFrameIn->update(&mIn);
			m_pFrameOut->getResizedOf(m_pFrameIn, scale, scale);
			mOut = *m_pFrameOut->getCMat();

			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("lowResolution: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total lowResolution: " << nTot);
}

void _Augment::crop(void)
{
	IF_(getDirFileList() <= 0);

	int nTot = 0;
	Mat mOut;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		for (int j = 0; j < m_nCrop; j++)
		{
			double rScaleW = 1.0 - m_dCrop * NormRand();
			int dSizeW = ((double) mIn.cols) * rScaleW;
			int dRandX = ((double) mIn.cols - dSizeW) * NormRand();

			double rScaleH = 1.0 - m_dCrop * NormRand();
			int dSizeH = ((double) mIn.rows) * rScaleH;
			int dRandY = ((double) mIn.rows - dSizeH) * NormRand();

			mOut = mIn(cv::Rect(dRandX, dRandY, dSizeW, dSizeH));

			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Crop: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total cropped: " << nTot);
}

void _Augment::flip(void)
{
	IF_(getDirFileList() <= 0);

	int nTot = 0;
	Mat mOut;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		Point2f pCenter = Point2f(mIn.cols / 2, mIn.rows / 2);
		Size s = Size(mIn.cols, mIn.rows);

		for (int j = -1; j < 2; j++)
		{
			cv::flip(mIn, mOut, j);

			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Flip: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total fliped: " << nTot);
}

void _Augment::contrast(void)
{
	IF_(getDirFileList() <= 0);

	int nTot = 0;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		Mat mOut;
		for (int j = 0; j < m_nContrast; j++)
		{
			mIn.convertTo(mOut, -1, 1.0 + m_dContrast*(NormRand()*2.0-1.0), 0);
			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Contrast: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total contrast changed: " << nTot);
}

void _Augment::brightness(void)
{
	IF_(getDirFileList() <= 0);

	int nTot = 0;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		Mat mOut;
		for (int j = 0; j < m_nBrightness; j++)
		{
			mIn.convertTo(mOut, -1, 1, m_dBrightness*(NormRand()*2.0-1.0));
			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Brightness: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total brightness changed: " << nTot);
}

void _Augment::histEqualize(void)
{
	IF_(getDirFileList() <= 0);

	int nTot = 0;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		Mat mOut;
        vector<Mat> vChannels;

        //Using reference code from:
        //https://opencv-srf.blogspot.jp/2013/08/histogram-equalization.html

		cv::cvtColor(mIn, mOut, CV_BGR2YCrCb); 			//change the color image from BGR to YCrCb format
		split(mOut, vChannels); 						//split the image into channels
		cv::equalizeHist(vChannels[0], vChannels[0]);   //equalize histogram on the 1st channel (Y)
		merge(vChannels,mIn); 							//merge 3 channels including the modified 1st channel into one image
        cv::cvtColor(mIn, mOut, CV_YCrCb2BGR); 			//change the color image from YCrCb to BGR format (to display image properly)

		cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Histogram equalization: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total histogram equalized: " << nTot);
}

void _Augment::blur(void)
{
	IF_(getDirFileList() <= 0);

	int nTot = 0;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		Mat mOut;
		for (int j = 0; j < m_nBlur; j++)
		{
			cv::blur(mIn, mOut, Size(m_dBlur*NormRand()+1,m_dBlur*NormRand()+1));
			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Blur: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total blurred: " << nTot);
}

void _Augment::gaussianBlur(void)
{
	IF_(getDirFileList() <= 0);

	int nTot = 0;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		Mat mOut;
		for (int j = 0; j < m_nGaussianBlur; j++)
		{
			int rX = m_dGaussianBlur*NormRand()+1;
			int rY = m_dGaussianBlur*NormRand()+1;

			if(rX%2==0)rX++;
			if(rY%2==0)rY++;

			cv::GaussianBlur(mIn, mOut, Size(rX,rY), 0, 0);
			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Gaussian Blur: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total gaussian blurred: " << nTot);
}

void _Augment::medianBlur(void)
{
	IF_(getDirFileList() <= 0);

	int nTot = 0;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		Mat mOut;
		for (int j = 0; j < m_nMedianBlur; j++)
		{
			int rK = m_dGaussianBlur*NormRand()+1;
			if(rK%2==0)rK++;

			cv::medianBlur(mIn, mOut, rK);
			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Median Blur: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total median blurred: " << nTot);
}

void _Augment::bilateralBlur(void)
{
	IF_(getDirFileList() <= 0);

	int nTot = 0;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		Mat mOut;
		for (int j = 0; j < m_nBilateralBlur; j++)
		{
			int rK = m_dBilateralBlur*NormRand()+1;
			if(rK%2==0)rK++;

			cv::bilateralFilter(mIn, mOut, rK, rK, rK);
			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Bilateral Blur: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total bilateral blurred: " << nTot);
}

void _Augment::tone(void)
{

}

void _Augment::move(void)
{

}


}

