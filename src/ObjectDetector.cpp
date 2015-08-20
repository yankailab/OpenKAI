/*
 * ObjectLocalizer.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "ObjectDetector.h"

ObjectDetector::ObjectDetector()
{
	// TODO Auto-generated constructor stub

}

ObjectDetector::~ObjectDetector()
{
	// TODO Auto-generated destructor stub
}

bool ObjectDetector::init(void)
{

	m_classifier.setup(
			"/Users/yankai/Documents/dev/caffe-master/models/bvlc_reference_caffenet/deploy.prototxt", //model_file,
			"/Users/yankai/Documents/dev/caffe-master/models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel", //trained_file,
			"/Users/yankai/Documents/dev/caffe-master/data/ilsvrc12/imagenet_mean.binaryproto", //mean_file,
			"/Users/yankai/Documents/dev/caffe-master/data/ilsvrc12/synset_words.txt" //label_file
			);

	m_tSleep = TRD_INTERVAL;

	LOG(INFO)<<"Object Detector Initialized";

	return true;
}

bool ObjectDetector::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode << " in ObjectDetector::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO) << "ObjectDetector.start()";

	return true;
}

void ObjectDetector::update(void)
{
	int tThreadBegin;

	while (m_bThreadON)
	{
		tThreadBegin = time(NULL);

		this->sleepThread(m_tSleep,0);//sleepThread can be woke up by this->wakeupThread()
	}

}

void ObjectDetector::stop(void)
{
	m_bThreadON = false;
	this->wakeupThread();
	pthread_join(m_threadID, NULL);

//	m_pVController->stop();

	LOG(INFO) << "ObjectDetector.stop()";
}

void ObjectDetector::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}

int ObjectDetector::detect(Mat pImg)
{
	m_predictions = m_classifier.Classify(pImg);

	for (size_t i = 0; i < g_predictions.size(); i++)
	{
		Prediction p = g_predictions[j];

//		std::cout << std::fixed << std::setprecision(4) << p.second << " - \"" << p.first << "\"" << std::endl;

		cv::putText(g_displayMat, String(p.first),
				Point(startPosH, startPosV + lineHeight * (++i)),
				FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	}

	return 5;
}



