/*
 *  Created on: Sept 28, 2015
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
	m_pFrame = NULL;
	m_frameID = 0;
	m_confidence_threshold = 0.0;

	m_pDN = NULL;
	m_nBox = 0;
	m_nClass = 0;

	m_bbCPU = NULL;
	m_bbCUDA = NULL;
	m_confCPU = NULL;
	m_confCUDA = NULL;
}

_DetectNet::~_DetectNet()
{
}

bool _DetectNet::init(Config* pConfig)
{
	CHECK_F(!this->_ThreadBase::init(pConfig));
	pConfig->m_pInst = this;

	m_pDN = detectNet::Create(detectNet::PEDNET_MULTI);
	NULL_F(m_pDN);

	m_nBox = m_pDN->GetMaxBoundingBoxes();
	m_nClass = m_pDN->GetNumClasses();

	CHECK_F(
			!cudaAllocMapped((void** )&m_bbCPU, (void** )&m_bbCUDA,
					m_nBox * sizeof(float4)));
	CHECK_F(
			!cudaAllocMapped((void** )&m_confCPU, (void** )&m_confCUDA,
					m_nBox * m_nClass * sizeof(float)));

	return true;

	//Setup model
	string detectNetDir = "";
	string modelFile;
	string trainedFile;
	string meanFile;
	string labelFile;
	string presetDir = "";

	F_INFO(pConfig->root()->o("APP")->v("presetDir", &presetDir));

	F_INFO(pConfig->v("dir", &detectNetDir));
	F_FATAL_F(pConfig->v("modelFile", &modelFile));
	F_FATAL_F(pConfig->v("trainedFile", &trainedFile));
	F_FATAL_F(pConfig->v("meanFile", &meanFile));
	F_FATAL_F(pConfig->v("labelFile", &labelFile));
	F_INFO(pConfig->v("minConfidence", &m_confidence_threshold));

	setup(detectNetDir + modelFile, detectNetDir + trainedFile,
			detectNetDir + meanFile, presetDir + labelFile);

	m_pFrame = new Frame();

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

	return true;
}

void _DetectNet::setup(const string& model_file, const string& trained_file,
		const string& mean_file, const string& label_file)
{

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
	CHECK_(m_pFrame->isNewerThan(pBGR));

	m_pFrame->getRGBAOf(pBGR);
	GpuMat* pGMat = m_pFrame->getGMat();

	CHECK_(!m_pDN->Detect((float* )pGMat->data, pGMat->cols, pGMat->rows,
					m_bbCPU, &m_nBox, m_confCPU));

	printf("%i bounding boxes detected\n", m_nBox);

//	int lastClass = 0;
//	int lastStart = 0;

	Rect bbox;

	for (int n = 0; n < m_nBox; n++)
	{
		const int nc = m_confCPU[n * 2 + 1];
		float* bb = m_bbCPU + (n * 4);

		printf("bounding box %i   (%f, %f)  (%f, %f)  w=%f  h=%f\n", n, bb[0],
				bb[1], bb[2], bb[3], bb[2] - bb[0], bb[3] - bb[1]);

//		if (nc != lastClass || n == (m_nBox - 1))
//		{
//			if (!net->DrawBoxes((float*) imgRGBA, (float*) imgRGBA,
//					camera->GetWidth(), camera->GetHeight(),
//					bbCUDA + (lastStart * 4), (n - lastStart) + 1, lastClass))
//				printf("detectnet-console:  failed to draw boxes\n");
//
//			lastClass = nc;
//			lastStart = n;
//		}

		 string name = "test";
		 bbox.x = bb[0];
		 bbox.y = bb[1];
		 bbox.width = bb[2] - bb[0];
		 bbox.height = bb[3] - bb[1];

		 m_pUniverse->addKnownObject(name, 0, NULL, &bbox, NULL);
	}

	/*if( font != NULL )
	 {
	 char str[256];
	 sprintf(str, "%05.2f%% %s", confidence * 100.0f, net->GetClassDesc(img_class));

	 font->RenderOverlay((float4*)imgRGBA, (float4*)imgRGBA, camera->GetWidth(), camera->GetHeight(),
	 str, 10, 10, make_float4(255.0f, 255.0f, 255.0f, 255.0f));
	 }*/

	/*
	 string name;
	 Rect bb;
	 Frame* pFrame;
	 unsigned int iClass;
	 unsigned int i;

	 pFrame = m_pStream->getBGRFrame();
	 if (pFrame->empty())
	 return;
	 if (!pFrame->isNewerThan(m_pFrame))
	 return;
	 m_pFrame->update(pFrame);

	 cv::cuda::GpuMat* pImg = m_pFrame->getGMat();
	 std::vector<vector<float> > detections = detect(m_pFrame);

	 //print the detection results
	 for (i = 0; i < detections.size(); ++i)
	 {
	 const vector<float>& d = detections[i];
	 // Detection format: [image_id, label, score, xmin, ymin, xmax, ymax].
	 //		CHECK_EQ(d.size(), 7);
	 //		float size = d.size();
	 const float score = d[2];

	 if (score < m_confidence_threshold)
	 continue;

	 iClass = static_cast<int>(d[1]) - 1;
	 if (iClass >= labels_.size())
	 continue;

	 name = labels_[iClass];
	 bb.x = d[3] * pImg->cols;
	 bb.y = d[4] * pImg->rows;
	 bb.width = d[5] * pImg->cols - bb.x;
	 bb.height = d[6] * pImg->rows - bb.y;

	 m_pUniverse->addKnownObject(name, safetyGrade_[iClass], NULL, &bb, NULL);

	 }
	 */
}

bool _DetectNet::draw(Frame* pFrame, iVec4* pTextPos)
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
