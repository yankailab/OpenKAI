/*
 * _Universe.h
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#ifndef SRC_NAVIGATION__UNIVERSE_H_
#define SRC_NAVIGATION__UNIVERSE_H_

#include "../Base/common.h"
#include "../Base/cv.h"
#include "../Base/_ThreadBase.h"
#include "../Stream/Frame.h"

#define NUM_OBJ 100

namespace kai
{

struct OBJ_CLASS
{
	string m_name;
	uint8_t m_safety;
};

struct OBJECT
{
	uint64_t m_frameID;	//0:vacant
	OBJ_CLASS* m_pClass;
	double	m_prob;
	vInt4	m_bbox;
	double	m_dist;
};

class _Universe: public _ThreadBase
{
public:
	_Universe();
	virtual ~_Universe();

	bool init(Config* pConfig);
	bool link(void);
	bool start(void);
	void reset(void);

	OBJECT* addObject(OBJ_CLASS* pClass, vInt4* pBbox, double dist, double prob);
	bool draw(Frame* pFrame, vInt4* pTextPos);

private:
	void updateObject(void);
	void deleteObject(int i);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Universe*) This)->update();
		return NULL;
	}

public:
	uint64_t m_frameID;
	uint64_t m_frameLifeTime;

	OBJECT m_pObj[NUM_OBJ];
	int m_numObj;
	int m_disparity;
	double m_objProbMin;

	vector<Mat> m_vMat;

};

} /* namespace kai */

#endif /* SRC_NAVIGATION__UNIVERSE_H_ */
