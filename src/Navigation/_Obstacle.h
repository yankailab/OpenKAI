/*
 * _Obstacle.h
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#ifndef SRC_NAVIGATION_OBSTACLE_H_
#define SRC_NAVIGATION_OBSTACLE_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Stream/_StreamBase.h"
#include "../AI/_ImageNet.h"

namespace kai
{

struct OBSTACLE
{
	vInt4 m_bbox;
	vInt2 m_camSize;
	double m_dist;
	double m_prob;
	int m_iClass;
	string m_name;
	uint8_t m_safety;
	vector<Point> m_contour;
	int64_t m_frameID;
};

class _Obstacle: public _ThreadBase
{
public:
	_Obstacle(void);
	virtual ~_Obstacle();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	int size(void);
	bool add(OBSTACLE* pNewObj);
	OBSTACLE* get(int i, int64_t frameID);
	OBSTACLE* getByClass(int iClass);

	void info(double* pRangeMin, double* pRangeMax, uint8_t* pOrientation);

private:
	void detect(void);
	double dist(Rect* pR);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Obstacle *) This)->update();
		return NULL;
	}

public:
	_StreamBase* m_pStream;
	_ImageNet* m_pIN;
	Frame*	m_pFrame;
	OBSTACLE* m_pObs;
	int m_nObs;
	int m_iObs;
	int64_t m_obsLifetime;

	double m_alertDist;
	double m_detectMinSize;
	double m_contourBlend;
	bool m_bDrawContour;

	double m_sizeName;
	double m_sizeDist;
	Scalar m_colName;
	Scalar m_colDist;
	Scalar m_colObs;

};

}

#endif
