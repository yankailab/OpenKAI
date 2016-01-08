

#include "_OpticalFlowDetector.h"

namespace kai
{
_OpticalFlowDetector::_OpticalFlowDetector()
{
	_ThreadBase();

	m_numHuman = 0;
	m_numCar = 0;
	m_pCamStream = NULL;

	scale = 1.05;
	nlevels = 13;
	gr_threshold = 8;
	hit_threshold = 1.4;

	win_width = 48;
	win_stride_width = 8;
	win_stride_height = 8;
	block_width = 16;
	block_stride_width = 8;
	block_stride_height = 8;
	cell_width = 8;
	nbins = 9;
}

_OpticalFlowDetector::~_OpticalFlowDetector()
{
}

bool _OpticalFlowDetector::init(JSON* pJson)
{
	string cascadeFile;
	CHECK_ERROR(pJson->getVal("CASCADE_FILE", &cascadeFile));
	m_pCascade = cuda::CascadeClassifier::create(cascadeFile);

	//HOG for Pedestrian detection
	string hogFile;
	CHECK_ERROR(pJson->getVal("HOG_FILE", &hogFile));

	Size win_stride(win_stride_width, win_stride_height);
	Size win_size(win_width, win_width * 2);
	Size block_size(block_width, block_width);
	Size block_stride(block_stride_width, block_stride_height);
	Size cell_size(cell_width, cell_width);

	m_pHumanHOG = cuda::HOG::create(win_size, block_size, block_stride,
			cell_size, nbins);
	m_pHumanHOG->setSVMDetector(m_pHumanHOG->getDefaultPeopleDetector());

	return true;
}

bool _OpticalFlowDetector::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode << " in OpticalFlowDetector::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO)<< "OpticalFlowDetector.start()";

	return true;
}

void _OpticalFlowDetector::update(void)
{
	int i;
	CamFrame* pFrame;
	int tThreadBegin;
	m_tSleep = TRD_INTERVAL_OPTFLOWDETECTOR;

	while (m_bThreadON)
	{
		tThreadBegin = time(NULL);

		if (!m_pCamStream)
			continue;
		pFrame = *(m_pCamStream->m_pFrameProcess);

		detect();

		//sleepThread can be woke up by this->wakeupThread()
		this->sleepThread(0, m_tSleep);
	}

}

void _OpticalFlowDetector::detect(void)
{
	int i, j;

	CamFrame* pFrame = *(m_pCamStream->m_pFrameProcess);

	GpuMat* pGray = m_pCamStream->m_pGrayL->getCurrentFrame();
	if (pGray->empty())
		return;

//	GpuMat* pBGRA = m_pCamStream->m_pBGRAL->getCurrentFrame();
//	if (pBGRA->empty())
//		return;

	//TODO: add frameID verify

	GpuMat cascadeGMat;
	vector<Rect> vRect;
	m_numHuman = 0;

	if (m_pCascade)
	{
		//	m_pCascade->setFindLargestObject(false);
		m_pCascade->setScaleFactor(1.2);
		//	m_pCascade->setMinNeighbors((filterRects || findLargestObject) ? 4 : 0);

		m_pCascade->detectMultiScale(*pGray, cascadeGMat);
		m_pCascade->convert(cascadeGMat, vRect);

		for (i = 0; i < vRect.size(); i++)
		{
			m_pHuman[m_numHuman].m_boundBox = vRect[i];
			m_numHuman++;
			if (m_numHuman == NUM_OPTICALFLOW_OBJECT)
			{
				break;
			}
		}
	}

	Size win_stride(win_stride_width, win_stride_height);

	m_pHumanHOG->setNumLevels(nlevels);
	m_pHumanHOG->setHitThreshold(hit_threshold);
	m_pHumanHOG->setWinStride(win_stride);
	m_pHumanHOG->setScaleFactor(scale);
	m_pHumanHOG->setGroupThreshold(gr_threshold);

//	m_pHumanHOG->detectMultiScale(*m_pBGRA, vRect);

//	m_numHuman = 0;
	for (i = 0; i < vRect.size(); i++)
	{
		m_pHuman[m_numHuman].m_boundBox = vRect[i];
		m_numHuman++;
		if (m_numHuman == NUM_OPTICALFLOW_OBJECT)
		{
			break;
		}
	}

}

void _OpticalFlowDetector::setFrame(_CamStream* pCam)
{
	if (!pCam)
		return;

	m_pCamStream = pCam;
	this->wakeupThread();
}

int _OpticalFlowDetector::getHuman(OPTICALFLOW_OBJECT** ppHuman)
{
	*ppHuman = m_pHuman;
	return m_numHuman;
}


}

