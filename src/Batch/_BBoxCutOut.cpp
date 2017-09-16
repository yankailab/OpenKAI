/*
 * _BBoxCutOut.cpp
 *
 *  Created on: Sept 16, 2017
 *      Author: yankai
 */

#include "_BBoxCutOut.h"

namespace kai
{

_BBoxCutOut::_BBoxCutOut()
{
}

_BBoxCutOut::~_BBoxCutOut()
{
}

bool _BBoxCutOut::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

//	KISSm(pK,baseDir);

	process();

	exit(0);
	return true;
}

bool _BBoxCutOut::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	return true;
}

void _BBoxCutOut::update(void)
{

}

void _BBoxCutOut::process()
{
}

/*
int _BBoxCutOut::readImgListToFloat(string fImgList, float *pData, float *pLabel, int nRead)
{
	ifstream ifs;
	string str;
	ifs.open(fImgList.c_str(), std::ios::in);
	if (!ifs)
	{
		LOG_E("Cannot open: " << fImgList);
		return 0;
	}

	int nImg = 0;
	while (getline(ifs, str))
	{
		vector<string> entry = splitBy(str, '\t');
		string fName = m_baseDir + entry[0];
		cout << "reading: " << fName << endl;

		cv::Mat img = cv::imread(fName);
		if(img.cols != m_width || img.rows != m_height)
		{
			cv::Mat rImg;
			cv::resize(img, rImg, cv::Size(m_width, m_height));
			img = rImg;
		}

		for (int y = 0; y < m_height; y++)
		{
			int iB = m_width * m_height * m_nChannel * nImg;

			for (int x = 0; x < m_width; x++)
			{
				int iA = y * img.cols + x + img.cols * img.rows;
				int iC = y * img.step + x * img.elemSize();

				pData[iA * 0 + iB] = img.data[iC + 0] - m_meanCol.x;
				pData[iA * 1 + iB] = img.data[iC + 1] - m_meanCol.y;
				pData[iA * 2 + iB] = img.data[iC + 2] - m_meanCol.z;
			}
		}

		for (int i = 0; i < m_outputDim; i++)
		{
			pLabel[nImg * m_outputDim + i] = (float)(stof(entry[i + 1]) * m_kLabel);
		}

		nImg++;
		if(nImg >= nRead)
			return nImg;
	}

	return nImg;
}
*/
}

