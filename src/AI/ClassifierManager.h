/*
 * ClassifierManager.h
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#ifndef SRC_AI_CLASSIFIERMANAGER_H_
#define SRC_AI_CLASSIFIERMANAGER_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Base/ThreadBase.h"
#include "../AI/NNClassifier.h"
#include "../Camera/CamStream.h"

#define TRD_INTERVAL_CLASSIFIER_MANAGER 10
#define NUM_OBJECT_NAME 5
#define NUM_OBJ 100

namespace kai
{

struct OBJECT
{
	string m_name[NUM_OBJECT_NAME];
	double m_prob[NUM_OBJECT_NAME];

	uint64_t		m_frameID;
	Mat			m_Mat;
	GpuMat		m_GMat;
	Rect			m_boundBox;
};

class ClassifierManager: public ThreadBase
{
public:
	ClassifierManager();
	virtual ~ClassifierManager();

	bool addObject(uint64_t frameID, Mat* pMat, Rect* pRect);
	bool addObject(uint64_t frameID, GpuMat* pGMat, Rect* pRect);

	bool init(JSON* pJson);
	bool start(void);
	bool complete(void);
	void stop(void);
	void waitForComplete(void);

private:
	pthread_t m_threadID;
	bool m_bThreadON;

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((ClassifierManager*) This)->update();
		return NULL;
	}

private:
	OBJECT m_pObjects[NUM_OBJ];
	int m_numObj;



	//Caffe classifier
	NNClassifier m_classifier;
	vector<Prediction> m_predictions;
	vector<vector<Prediction> > m_vPredictions;
};

} /* namespace kai */

#endif /* SRC_AI_CLASSIFIERMANAGER_H_ */
