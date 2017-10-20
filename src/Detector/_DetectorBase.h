/*
 * _detectorBase.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_Detector__DETECTORBASE_H_
#define OPENKAI_SRC_Detector__DETECTORBASE_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Vision/_VisionBase.h"

#define N_CLASS 128
#define N_OBJ 1024

namespace kai
{

enum detectorMode
{
	thread, noThread
};

struct OBJECT
{
	vDouble4 m_fBBox;
	vInt4 m_bbox;
	vInt2 m_camSize;
	double m_dist;
	double m_prob;
	int m_iClass;
	string m_name;
	vector<Point> m_contour;
	int64_t m_frameID;
	uint8_t m_safety;
	bool m_bCluster;

	void init(void)
	{
		m_fBBox.init();
		m_bbox.init();
		m_camSize.init();
		m_dist = -1.0;
		m_prob = 0;
		m_iClass = -1;
		m_name = "";
		m_frameID = -1;
		m_safety = -1;
		m_bCluster = false;
	}

	void f2iBBox(void)
	{
		m_bbox.x = m_fBBox.x * m_camSize.x;
		m_bbox.z = m_fBBox.z * m_camSize.x;
		m_bbox.y = m_fBBox.y * m_camSize.y;
		m_bbox.w = m_fBBox.w * m_camSize.y;
	}

	void i2fBBox(void)
	{
		double base;
		base = 1.0/m_camSize.x;
		m_fBBox.x = m_bbox.x * base;
		m_fBBox.z = m_bbox.z * base;
		base = 1.0/m_camSize.y;
		m_fBBox.y = m_bbox.y * base;
		m_fBBox.w = m_bbox.w * base;
	}
};

struct OBJECT_ARRAY
{
	OBJECT m_pObj[N_OBJ];
	int m_nObj;

	void reset(void)
	{
		m_nObj = 0;
	}

	OBJECT* add(OBJECT* pO)
	{
		NULL_N(pO);
		IF_N(m_nObj >= N_OBJ);

		m_pObj[m_nObj++] = *pO;

		return &m_pObj[m_nObj-1];
	}

	OBJECT* at(int i)
	{
		IF_N(i >= m_nObj);
		return &m_pObj[i];
	}

	int size(void)
	{
		return m_nObj;
	}
};

struct OBJECT_DARRAY
{
	OBJECT_ARRAY m_objArr[2];
	OBJECT_ARRAY* m_pPrev;
	OBJECT_ARRAY* m_pNext;
	int m_iSwitch;

	void reset(void)
	{
		m_iSwitch = 0;
		update();
		m_pPrev->reset();
		m_pNext->reset();
	}

	void update(void)
	{
		m_iSwitch = 1 - m_iSwitch;
		m_pPrev = &m_objArr[m_iSwitch];
		m_pNext = &m_objArr[1 - m_iSwitch];

		m_pNext->reset();
	}

	OBJECT* add(OBJECT* pO)
	{
		return m_pNext->add(pO);
	}

	OBJECT* at(int i)
	{
		return m_pPrev->at(i);
	}

	int size(void)
	{
		return m_pPrev->size();
	}
};

struct CLASS_DRAW
{
	Scalar  m_colorBBox;
	bool	m_bDraw;
	int		m_n;
	string  m_name;

	void init(void)
	{
		m_colorBBox = Scalar(255,255,255);
		m_bDraw = true;
		m_n = 0;
		m_name = "";
	}
};

class _DetectorBase: public _ThreadBase
{
public:
	_DetectorBase();
	virtual ~_DetectorBase();

	virtual bool init(void* pKiss);
	virtual bool link(void);
	virtual bool draw(void);

	void bSetActive(bool bActive);
	OBJECT* add(OBJECT* pNewObj);
	OBJECT* at(int i);
	void addOrUpdate(OBJECT* pNewObj);
	int size(void);
	void mergeDetector(void);

public:
	bool m_bActive;
	_VisionBase* m_pVision;
	_DetectorBase* m_pDetIn;
	double m_overlapMin;
	OBJECT_DARRAY m_obj;

	string m_modelFile;
	string m_trainedFile;
	string m_meanFile;
	string m_labelFile;

	double m_defaultDrawTextSize;
	Scalar m_defaultDrawColor;
	double m_contourBlend;
	bool m_bDrawContour;

	vector<CLASS_DRAW> m_vClassDraw;
	vInt3 m_classDrawPos;
	double m_minConfidence;
	detectorMode m_mode;


};

}

#endif
