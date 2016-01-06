/*
 * ClassifierManager.h
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#ifndef SRC_AI__CLASSIFIERMANAGER_H_
#define SRC_AI__CLASSIFIERMANAGER_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Base/_ThreadBase.h"
#include "../AI/NNClassifier.h"
#include "../AI/SegNet.h"
#include "../Camera/CamFrame.h"

#define TRD_INTERVAL_CLASSIFIER_MANAGER 0
#define NUM_OBJECT_NAME 5
#define NUM_OBJ 100
#define NUM_DETECT_BATCH 10

namespace kai {

struct OBJECT {
	string m_name[NUM_OBJECT_NAME];
	double m_prob[NUM_OBJECT_NAME];

	uint16_t m_status;
	uint64_t m_frameID;

	Mat m_Mat;
	GpuMat m_GMat;
	Rect m_boundBox;
	vector<Point> m_vContours;

};

class _ClassifierManager: public _ThreadBase {
public:
	_ClassifierManager();
	virtual ~_ClassifierManager();

	bool addObject(uint64_t frameID, Mat* pMat, Rect* pRect,
			vector<Point>* pContour);
	void classifyObject(void);

	void updateFrame(CamFrame* pFrame);

	bool init(JSON* pJson);
	bool start(void);

private:
	void deleteObject(int i);

	void update(void);
	static void* getUpdateThread(void* This) {
		((_ClassifierManager*) This)->update();
		return NULL;
	}

public:
	uint64_t m_globalFrameID;
	uint64_t m_frameLifeTime;
	OBJECT m_pObjects[NUM_OBJ];
	int m_numObj;
	int m_disparity;
	double m_objProbMin;

	vector<Mat> m_vMat;

	//Caffe classifier
	NNClassifier m_classifier;
	vector<Prediction> m_predictions;
	vector<vector<Prediction> > m_vPredictions;
	int m_numBatch;

	//SegNet
	SegNet m_segnet;
	Mat m_segnetMat;
	Mat m_segment;
	CamFrame* m_pFrame;
};

} /* namespace kai */

#endif /* SRC_AI__CLASSIFIERMANAGER_H_ */
