/*
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */
#include "_DNNdetect.h"

#ifdef USE_OPENCV

namespace kai
{

_DNNdetect::_DNNdetect()
{
	m_dnnType = dnn_yolo;
	m_thr = 0.5;
	m_nms = 0.4;
	m_nW = 416;
	m_nH = 416;
	m_bSwapRB = true;
	m_vMean.init();
	m_scale = 1.0 / 255.0;
	m_iClassDraw = -1;

	m_iBackend = cv::dnn::DNN_BACKEND_OPENCV;
	m_iTarget = cv::dnn::DNN_TARGET_CPU;
}

_DNNdetect::~_DNNdetect()
{
}

bool _DNNdetect::init(void *pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("thr", &m_thr);
	pK->v("nms", &m_nms);
	pK->v("nW", &m_nW);
	pK->v("nH", &m_nH);
	pK->v("iBackend", &m_iBackend);
	pK->v("iTarget", &m_iTarget);
	pK->v("bSwapRB", &m_bSwapRB);
	pK->v("scale", &m_scale);
	pK->v("iClassDraw", &m_iClassDraw);
	pK->v("vMean", &m_vMean);

	string n;
	pK->v("dnnType", &n);
	if (n == "caffe")
		m_dnnType = dnn_caffe;
	else if (n == "yolo")
		m_dnnType = dnn_yolo;
	else if (n == "tf")
		m_dnnType = dnn_tf;

	if (m_dnnType == dnn_yolo)
	{
		m_net = readNetFromDarknet(m_fModel, m_fWeight);
	}
	else if (m_dnnType == dnn_caffe)
	{
		m_net = readNetFromCaffe(m_fModel, m_fWeight);
	}
	else if (m_dnnType == dnn_tf)
	{
		m_net = readNetFromTensorflow(m_fWeight, m_fModel);
	}

	IF_Fl(m_net.empty(), "read Net failed");

	m_net.setPreferableBackend(m_iBackend);
	m_net.setPreferableTarget(m_iTarget);

	vector<int> outLayers = m_net.getUnconnectedOutLayers();
	vector<String> layersNames = m_net.getLayerNames();
	m_vLayerName.resize(outLayers.size());

	for (size_t i = 0; i < outLayers.size(); i++)
		m_vLayerName[i] = layersNames[outLayers[i] - 1];

	return true;
}

bool _DNNdetect::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _DNNdetect::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		if (check() >= 0)
		{

			if (m_dnnType == dnn_yolo)
				detectYolo();
			else if (m_dnnType == dnn_tf || m_dnnType == dnn_caffe)
				detect();

			if (m_pT->bGoSleep())
				m_pU->m_pPrev->clear();
		}

		m_pT->autoFPSto();
	}
}

int _DNNdetect::check(void)
{
	NULL__(m_pU, -1);
	NULL__(m_pV, -1);
	Frame *pBGR = m_pV->BGR();
	NULL__(pBGR, -1);
	IF__(pBGR->bEmpty(), -1);
	IF__(pBGR->tStamp() <= m_fBGR.tStamp(), -1);

	return this->_DetectorBase::check();
}

void _DNNdetect::detectYolo(void)
{
	Frame *pBGR = m_pV->BGR();
	m_fBGR.copy(*pBGR);
	Mat mIn = *m_fBGR.m();

	m_blob = blobFromImage(mIn, m_scale, Size(m_nW, m_nH),
			Scalar(m_vMean.x, m_vMean.y, m_vMean.z), m_bSwapRB, false);
	m_net.setInput(m_blob);

	// Runs the forward pass to get output of the output layers
	vector<Mat> vO;
#if CV_VERSION_MAJOR > 3
	m_net.forward(vO, m_vLayerName);
#endif

	// Remove the bounding boxes with low confidence
	vector<int> vClassID;
	vector<float> vConfidence;
	vector<Rect> vRect;

	for (size_t i = 0; i < vO.size(); i++)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float *pData = (float*) vO[i].data;
		for (int j = 0; j < vO[i].rows; ++j, pData += vO[i].cols)
		{
			Mat mScore = vO[i].row(j).colRange(5, vO[i].cols);
			Point pClassID;
			double conf;

			// Get the value and location of the maximum score
			cv::minMaxLoc(mScore, 0, &conf, 0, &pClassID);
			IF_CONT(conf < m_thr);

			int cX = (int) (pData[0] * mIn.cols);
			int cY = (int) (pData[1] * mIn.rows);
			int w = (int) (pData[2] * mIn.cols);
			int h = (int) (pData[3] * mIn.rows);
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
	float kx = 1.0 / mIn.cols;
	float ky = 1.0 / mIn.rows;

	for (size_t i = 0; i < vIndex.size(); i++)
	{
		int idx = vIndex[i];

		_Object o;
		o.init();
//		o.m_tStamp = m_pT->getTfrom();
		o.setTopClass(vClassID[idx], (float) vConfidence[idx]);
		o.setBB2D(rect2BB<vFloat4>(vRect[idx]));
		o.scale(kx, ky);

		m_pU->add(o);
		LOG_I("Class: " + i2str(o.getTopClass()));
	}

	m_pU->updateObj();
}

void _DNNdetect::detect(void)
{
	Frame *pBGR = m_pV->BGR();
	m_fBGR.copy(*pBGR);
	Mat mIn = *m_fBGR.m();

	m_blob = blobFromImage(mIn, m_scale, Size(m_nW, m_nH),
			Scalar(m_vMean.x, m_vMean.y, m_vMean.z), m_bSwapRB, false);

	m_net.setInput(m_blob, "data");
	cv::Mat mDet = m_net.forward("detection_out");
	cv::Mat dMat(mDet.size[2], mDet.size[3], CV_32F, mDet.ptr<float>());

	for (int i = 0; i < dMat.rows; i++)
	{
		float conf = dMat.at<float>(i, 2);
		IF_CONT(conf < m_thr);

		vFloat4 bb;
		bb.x = dMat.at<float>(i, 3);
		bb.y = dMat.at<float>(i, 4);
		bb.z = dMat.at<float>(i, 5);
		bb.w = dMat.at<float>(i, 6);
		bb.constrain(0.0, 1.0);

		_Object o;
		o.init();
//		o.m_tStamp = m_pT->getTfrom();
		o.setTopClass(0.0, 1.0);
		o.setBB2D(bb);

		m_pU->add(o);
		LOG_I("Class: " + i2str(o.getTopClass()));
	}

	m_pU->updateObj();
}

void _DNNdetect::draw(void)
{
	IF_(check() < 0);

	this->_ModuleBase::draw();
	IF_(!checkWindow());
	Mat *pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	Scalar col = Scalar(0, 0, 255);

	_Object *pO;
	int i = 0;
	while ((pO = m_pU->get(i++)) != NULL)
	{
		int iClass = pO->getTopClass();
		IF_CONT(m_iClassDraw >= 0 && iClass != m_iClassDraw);
		IF_CONT(iClass < 0);

		Rect r = bb2Rect(pO->getBB2Dscaled(pMat->cols, pMat->rows));
		rectangle(*pMat, r, col, 1);

		if (iClass < m_nClass)
		{
			string oName = m_vClass[iClass].m_name;
			if (oName.length() > 0)
			{
				putText(*pMat, oName, Point(r.x + 15, r.y + 25),
						FONT_HERSHEY_SIMPLEX, 1.0, col, 2);
			}
		}
	}
}

}
#endif
