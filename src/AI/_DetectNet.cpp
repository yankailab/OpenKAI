/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_DetectNet.h"

#ifdef USE_TENSORRT


namespace kai
{
_DetectNet::_DetectNet()
{
	_ThreadBase();

	num_channels_ = 0;
	m_pUniverse = NULL;
	m_pStream = NULL;
	m_pRGBA = NULL;
	m_pRGBAf = NULL;
	m_frameID = 0;
	m_confidence_threshold = 0.0;

	m_pDN = NULL;
	m_nBox = 0;
	m_nBoxMax = 0;
	m_nClass = 0;

	m_bbCPU = NULL;
	m_bbCUDA = NULL;
	m_confCPU = NULL;
	m_confCUDA = NULL;

	m_className = "target";

	modelFile="";
	trainedFile="";
	meanFile="";
	labelFile="";

}

_DetectNet::~_DetectNet()
{
}

bool _DetectNet::init(Config* pConfig)
{
	CHECK_F(!this->_ThreadBase::init(pConfig));
	pConfig->m_pInst = this;

	m_pRGBA = new Frame();
	m_pRGBAf = new Frame();

	//Setup model
	string detectNetDir = "";
	string presetDir = "";

	F_INFO(pConfig->root()->o("APP")->v("presetDir", &presetDir));

	F_INFO(pConfig->v("dir", &detectNetDir));
	F_FATAL_F(pConfig->v("modelFile", &modelFile));
	F_FATAL_F(pConfig->v("trainedFile", &trainedFile));
	F_FATAL_F(pConfig->v("meanFile", &meanFile));
	F_FATAL_F(pConfig->v("labelFile", &labelFile));
	F_INFO(pConfig->v("minConfidence", &m_confidence_threshold));
	F_INFO(pConfig->v("className", &m_className));


	modelFile = detectNetDir + modelFile;
	trainedFile = detectNetDir + trainedFile;
	meanFile = detectNetDir + meanFile;
	labelFile = presetDir + labelFile;




	return true;
}

bool _DetectNet::link(void)
{
	NULL_F(m_pConfig);

	string iName = "";
	F_ERROR_F(m_pConfig->v("_Stream", &iName));
	m_pStream = (_Stream*) (m_pConfig->root()->getChildInstByName(&iName));
	F_ERROR_F(m_pConfig->v("_Universe", &iName));
	m_pUniverse = (_Universe*) (m_pConfig->root()->getChildInstByName(&iName));

	//TODO: link my variables to Automaton

	printf("_DetectNet link complete\n");

	return true;
}

bool _DetectNet::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _DetectNet::update(void)
{

	m_pDN = detectNet::Create(modelFile.c_str(),
				  trainedFile.c_str(),
				  meanFile.c_str(),
				  m_confidence_threshold );
	NULL_(m_pDN);

	m_nBoxMax = m_pDN->GetMaxBoundingBoxes();
	m_nClass = m_pDN->GetNumClasses();

	CHECK_(!cudaAllocMapped((void** )&m_bbCPU, (void** )&m_bbCUDA,
					m_nBoxMax * sizeof(float4)));
	CHECK_(!cudaAllocMapped((void** )&m_confCPU, (void** )&m_confCUDA,
					m_nBoxMax * m_nClass * sizeof(float)));

	printf("_DetectNet setup complete\n");

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		m_frameID = get_time_usec();

		detectFrame();

		this->autoFPSto();
	}

}

void _DetectNet::detectFrame(void)
{
	NULL_(m_pStream);
	NULL_(m_pUniverse);
	NULL_(m_pDN);

	Frame* pBGR = m_pStream->getBGRFrame();
	NULL_(pBGR);
	CHECK_(pBGR->empty());
	CHECK_(m_pRGBA->isNewerThan(pBGR));

	
	m_pRGBA->getRGBAOf(pBGR);
	GpuMat* pGMat = m_pRGBA->getGMat();
	CHECK_(pGMat->empty());

	GpuMat fGMat;
	pGMat->convertTo(fGMat,CV_32FC4);


/*	m_pRGBA->getRGBAOf(pBGR);
	m_pRGBAf->get32FC4Of(m_pRGBA);
	GpuMat* fGMat = m_pRGBA->getGMat();
*/
	m_nBox = m_nBoxMax;

	CHECK_(!m_pDN->Detect((float*)fGMat.data, fGMat.cols, fGMat.rows, m_bbCPU, &m_nBox, m_confCPU));
	printf("%i bounding boxes detected\n", m_nBox);

	Rect bbox;

	for (int n = 0; n < m_nBox; n++)
	{
		const int nc = m_confCPU[n * 2 + 1];
		float* bb = m_bbCPU + (n * 4);

		printf("bounding box %i   (%f, %f)  (%f, %f)  w=%f  h=%f\n", n, bb[0],
				bb[1], bb[2], bb[3], bb[2] - bb[0], bb[3] - bb[1]);

		 bbox.x = (int)bb[0];
		 bbox.y = (int)bb[1];
		 bbox.width = (int)(bb[2] - bb[0]);
		 bbox.height = (int)(bb[3] - bb[1]);

		 m_pUniverse->addKnownObject(m_className, 0, NULL, &bbox, NULL);
	}





/*

			int lastClass = 0;
			int lastStart = 0;
			
			for( int n=0; n < m_nBox; n++ )
			{
				const int nc = m_confCPU[n*2+1];
				float* bb = m_bbCPU + (n * 4);
				
				printf("bounding box %i   (%f, %f)  (%f, %f)  w=%f  h=%f\n", n, bb[0], bb[1], bb[2], bb[3], bb[2] - bb[0], bb[3] - bb[1]); 
				
				if( nc != lastClass || n == (m_nBox - 1) )
				{
//					if( !net->DrawBoxes((float*)imgFC, (float*)imgFC, camera->GetWidth(), camera->GetHeight(), bbCUDA+(lastStart * 4), (n - lastStart) + 1, lastClass) )
//					bool DrawBoxes( float* input, float* output, uint32_t width, uint32_t height, const float* boundingBoxes, int numBoxes, int classIndex=0 );

					float* pbb = m_bbCPU + (lastStart*4);


					 bbox.x = pbb[0];		
					 bbox.y = pbb[1];
					 bbox.width = (pbb[2] - pbb[0]);
					 bbox.height = (pbb[3] - pbb[1]);

					 m_pUniverse->addKnownObject(m_className, 0, NULL, &bbox, NULL);

					
					lastClass = nc;
					lastStart = n;
				}
			}
*/
}

bool _DetectNet::draw(Frame* pFrame, vInt4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	putText(*pFrame->getCMat(), "DetectNet FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}

}

#endif
