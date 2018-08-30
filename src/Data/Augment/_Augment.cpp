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
	m_nThread = 1;
	m_bConvertFormat = false;
}

_Augment::~_Augment()
{
}

bool _Augment::init(void* pKiss)
{
	IF_F(!this->_DataBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, nThread);
	KISSm(pK, bConvertFormat);

	if(m_nThread <= 0)m_nThread = 1;

	m_vFilter.clear();
	string pFilterIn[AUGMENT_N_FILTER];
	int nCmd = pK->array("filter", pFilterIn, AUGMENT_N_FILTER);
	for (int i = 0; i < nCmd; i++)
	{
		m_vFilter.push_back(pFilterIn[i]);
	}

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
	int i,j,k;
	Kiss* pK = (Kiss*) m_pKiss;

	for (i = 0; i < m_vFilter.size(); i++)
	{
		LOG_I("- Filter stage " + i2str(i) + ": " + m_vFilter[i]);

		//create filter threads
		string filter = m_vFilter[i];

		Kiss* pFK = pK->o(filter);
		NULL_l(pFK,"Filter Kiss not found");

		for(j=0; j<m_nThread; j++)
		{
			m_ppFB[j] = createFilterThread(filter);
			NULL_(m_ppFB[j]);
			IF_l(!m_ppFB[j]->init(pFK),"Filter thread init failed");
		}

		//get input file list
		IF_(getDirFileList() <= 0);

		if (m_bConvertFormat)
		{
			for(j=0; j<m_vFileIn.size(); j++)
			{
				string fNameIn = m_vFileIn[i];
				IF_CONT(getExtension(fNameIn) == m_extOut);

				Mat mIn = cv::imread(fNameIn.c_str());
				IF_CONT(mIn.empty());

				string dirNameIn = getFileDir(fNameIn);
				cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
				remove(fNameIn.c_str());
			}

			IF_(getDirFileList() <= 0);
		}

		LOG_I("- nFiles found: " + i2str(m_vFileIn.size()));

		//allocate to filter threads
		LOG_I("- nThreads created: " + i2str(m_nThread));

		int nTFile = (m_vFileIn.size() / m_nThread) + 1;
		vector<string> vTFile;
		int iFile = 0;
		int nThread = 0;

		for(j=0; j<m_nThread; j++)
		{
			if(iFile >= m_vFileIn.size())
			{
				break;
			}

			vTFile.clear();
			for(k=0; k<nTFile; k++)
			{
				vTFile.push_back(m_vFileIn[iFile++]);
				if(iFile >= m_vFileIn.size())break;
			}

			_FilterBase* pFB = m_ppFB[j];
			pFB->setFileList(vTFile);
			IF_l(!pFB->start(), "Filter thread start failed");
			nThread++;
			LOG_I(" - iThread started: " + i2str(j));
		}

		LOG_I("- nThreads started: " + i2str(nThread));

		//wait for filter threads to complete
		int nComplete = 0;
		while(nComplete < nThread)
		{
			this->sleepTime(USEC_1SEC);
			nComplete = 0;
			for(j=0; j<nThread; j++)
				nComplete += (m_ppFB[j]->bComplete())?1:0;
		}

		for(j=0; j<m_nThread; j++)
		{
			DEL(m_ppFB[j]);
		}

	}

	LOG_I("- All filters completed");
	exit(0);
}

_FilterBase* _Augment::createFilterThread(string& filter)
{
	_FilterBase* pFB = NULL;

	if (filter == "rotate")
		return new _filterRotate();
	else if (filter == "shrink")
		return new _filterShrink();
	else if (filter == "crop")
		return new _filterCrop();
	else if (filter == "flip")
		return new _filterFlip();
	else if (filter == "lowResolution")
		return new _filterLowResolution();
	else if (filter == "noise")
		return new _filterNoise();
	else if (filter == "contrast")
		return new _filterContrast();
	else if (filter == "brightness")
		return new _filterBrightness();
	else if (filter == "histEqualize")
		return new _filterHistEqualize();
	else if (filter == "blur")
		return new _filterBlur();
	else if (filter == "gaussianBlur")
		return new _filterGaussianBlur();
	else if (filter == "medianBlur")
		return new _filterMedianBlur();
	else if (filter == "bilateralBlur")
		return new _filterBilateralBlur();
	else if (filter == "saturation")
		return new _filterSaturation();
	else if (filter == "tone")
		return new _filterTone();

	LOG_E("Unrecognized augment filter: " + filter);
	return NULL;

}

}

