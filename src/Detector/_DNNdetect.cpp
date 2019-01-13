/*
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */
#include "_DNNdetect.h"

namespace kai
{

_DNNdetect::_DNNdetect()
{
	m_thr = 0.5;
	m_nms = 0.4;
	m_nW = 416;
	m_nH = 416;
	m_bSwapRB = true;
	m_vMean.init();
	m_scale = 1.0/255.0;

	m_iBackend = cv::dnn::DNN_BACKEND_OPENCV;
	m_iTarget = cv::dnn::DNN_TARGET_CPU;
}

_DNNdetect::~_DNNdetect()
{
}

bool _DNNdetect::init(void* pKiss)
{
	IF_F(!this->_ObjectBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, thr);
	KISSm(pK, nms);
	KISSm(pK, nW);
	KISSm(pK, nH);
	KISSm(pK, iBackend);
	KISSm(pK, iTarget);
	KISSm(pK, bSwapRB);
	KISSm(pK, scale);
	pK->v("meanB",&m_vMean.x);
	pK->v("meanG",&m_vMean.y);
	pK->v("meanR",&m_vMean.z);

	m_net = readNetFromDarknet(m_modelFile, m_trainedFile);
	IF_Fl(m_net.empty(), "read Net failed");

	m_net.setPreferableBackend(m_iBackend);
	m_net.setPreferableTarget(m_iTarget);

	//Get the indices of the output layers, i.e. the layers with unconnected outputs
	vector<int> outLayers = m_net.getUnconnectedOutLayers();
	//get the names of all the layers in the network
	vector<String> layersNames = m_net.getLayerNames();
	// Get the names of the output layers in names
	m_vLayerName.resize(outLayers.size());

	for (size_t i = 0; i < outLayers.size(); i++)
		m_vLayerName[i] = layersNames[outLayers[i] - 1];

	return true;
}

bool _DNNdetect::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _DNNdetect::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		IF_CONT(!detect());

		m_obj.update();

		if (m_bGoSleep)
		{
			m_obj.m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

int _DNNdetect::check(void)
{
	NULL__(m_pVision, -1);
	Frame* pBGR = m_pVision->BGR();
	NULL__(pBGR, -1);
	IF__(pBGR->bEmpty(), -1);
	IF__(pBGR->tStamp() <= m_BGR.tStamp(), -1);

	return 0;
}

bool _DNNdetect::detect(void)
{
	IF_F(check() < 0);

	Frame* pBGR = m_pVision->BGR();
	m_BGR.copy(*pBGR);
	Mat mIn = *m_BGR.m();

	vInt4 iRoi;
	iRoi.x = mIn.cols * m_roi.x;
	iRoi.y = mIn.rows * m_roi.y;
	iRoi.z = mIn.cols * m_roi.z;
	iRoi.w = mIn.rows * m_roi.w;
	Rect rRoi;
	vInt42rect(iRoi, rRoi);
	Mat mBGR = mIn(rRoi);

	m_blob = blobFromImage(mBGR,
							m_scale,
							Size(m_nW, m_nH),
							Scalar(m_vMean.x, m_vMean.y, m_vMean.z),
							m_bSwapRB,
							false);
	m_net.setInput(m_blob);

	// Runs the forward pass to get output of the output layers
	vector<Mat> vO;
	m_net.forward(vO, m_vLayerName);

	// Remove the bounding boxes with low confidence
	vector<int> vClassID;
	vector<float> vConfidence;
	vector<Rect> vRect;

	for (size_t i = 0; i < vO.size(); i++)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float* pData = (float*) vO[i].data;
		for (int j = 0; j < vO[i].rows; ++j, pData += vO[i].cols)
		{
			Mat mScore = vO[i].row(j).colRange(5, vO[i].cols);
			Point pClassID;
			double conf;

			// Get the value and location of the maximum score
			minMaxLoc(mScore, 0, &conf, 0, &pClassID);
			IF_CONT(conf < m_thr);

			int cX = (int) (pData[0] * mBGR.cols);
			int cY = (int) (pData[1] * mBGR.rows);
			int w = (int) (pData[2] * mBGR.cols);
			int h = (int) (pData[3] * mBGR.rows);
			int l = cX - w / 2;
			int t = cY - h / 2;

			vClassID.push_back(pClassID.x);
			vConfidence.push_back((float) conf);
			vRect.push_back(Rect(l, t, w, h));
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with lower confidences
	vector<int> vIndex;
	NMSBoxes(vRect, vConfidence, m_thr, m_nms, vIndex);
	vInt2 cSize;
	cSize.x = mIn.cols;
	cSize.y = mIn.rows;

	for (size_t i = 0; i < vIndex.size(); i++)
	{
		int idx = vIndex[i];
		Rect rBB = vRect[idx];

		OBJECT obj;
		obj.init();
		obj.m_tStamp = m_tStamp;
		obj.setTopClass(vClassID[idx], (double)vConfidence[idx]);

		vInt4 iBB;
		iBB.x = rRoi.x + rBB.x;
		iBB.y = rRoi.y + rBB.y;
		iBB.z = iBB.x + rBB.width;
		iBB.w = iBB.y + rBB.height;
		obj.setBB(iBB, cSize);

		this->add(&obj);
		LOG_I("Class: " + i2str(obj.m_topClass));
	}

	return true;
}

bool _DNNdetect::draw(void)
{
	IF_F(!this->_ObjectBase::draw());

	return true;
}

}
