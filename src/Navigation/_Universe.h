/*
 * _Universe.h
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#ifndef SRC_NAVIGATION__UNIVERSE_H_
#define SRC_NAVIGATION__UNIVERSE_H_

#include "../AI/_Caffe.h"
#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Base/_ThreadBase.h"

#define NUM_OBJECT_NAME 5
#define NUM_OBJ 100
#define NUM_DETECT_BATCH 1


namespace kai
{

struct OBJECT
{
	string m_name[NUM_OBJECT_NAME];
	double m_prob[NUM_OBJECT_NAME];

	uint16_t m_status;
	uint64_t m_frameID;
	int	m_safetyGrade;

	Mat m_Mat;
	GpuMat m_GMat;
	Rect m_boundBox;
	vector<Point> m_vContours;

};

class _Universe: public _ThreadBase
{
public:
	_Universe();
	virtual ~_Universe();

	OBJECT* addUnknownObject(Mat* pMat, Rect* pRect, vector<Point>* pContour);
	OBJECT* addKnownObject(string name, int safetyGrade, Mat* pMat, Rect* pRect, vector<Point>* pContour);
	void classifyObject(void);
	bool draw(Frame* pFrame, iVec4* pTextPos);

	bool init(Config* pConfig);
	bool start(void);
	void reset(void);

private:
	void deleteObject(int i);

	void update(void);
	static void* getUpdateThread(void* This) {
		((_Universe*) This)->update();
		return NULL;
	}

public:
	uint64_t m_frameID;
	uint64_t m_frameLifeTime;
	OBJECT m_pObjects[NUM_OBJ];
	int m_numObj;
	int m_disparity;
	double m_objProbMin;

	vector<Mat> m_vMat;

	//Caffe classifier
	_Caffe m_caffe;
	vector<Prediction> m_predictions;
	vector<vector<Prediction> > m_vPredictions;
	int m_numBatch;

};

} /* namespace kai */

#endif /* SRC_NAVIGATION__UNIVERSE_H_ */
