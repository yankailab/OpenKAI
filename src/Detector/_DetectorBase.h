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

namespace kai
{

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
	bool m_bClassify;

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
		m_bClassify = false;
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
	OBJECT* m_pObj;
	int m_nObj;
	int m_iObj;

	bool init(int nObj)
	{
		IF_F(nObj<=0);

		m_nObj = nObj;
		m_iObj = 0;
		m_pObj = new OBJECT[m_nObj];

		for (int i = 0; i < m_nObj; i++)
		{
			m_pObj[i].init();
		}

		return true;
	}

	void reset(void)
	{
		m_iObj = 0;
	}

	OBJECT* add(OBJECT* pO)
	{
		NULL_N(pO);
		IF_N(m_iObj >= m_nObj);

		m_pObj[m_iObj++] = *pO;

		return &m_pObj[m_iObj];
	}

	OBJECT* at(int i)
	{
		IF_N(i >= m_iObj);
		return &m_pObj[i];
	}

	int size(void)
	{
		return m_iObj;
	}

	void release()
	{
		DEL(m_pObj);
		m_pObj = NULL;
	}

};

struct OBJECT_DARRAY
{
	OBJECT_ARRAY m_objArr[2];
	OBJECT_ARRAY* m_pPrev;
	OBJECT_ARRAY* m_pNext;
	int m_iSwitch;

	bool init(int nObj)
	{
		IF_F(nObj<=0);

		m_iSwitch = 0;
		update();

		IF_F(!m_pPrev->init(nObj));
		IF_F(!m_pNext->init(nObj));

		return true;
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

	void release()
	{
		m_objArr[0].release();
		m_objArr[1].release();
	}
};

class _DetectorBase: public _ThreadBase
{
public:
	_DetectorBase();
	virtual ~_DetectorBase();

	virtual bool init(void* pKiss);
	virtual bool link(void);
	virtual bool start(void);
	virtual bool draw(void);

	OBJECT* add(OBJECT* pNewObj);
	OBJECT* at(int i);
	void addOrUpdate(OBJECT* pNewObj);
	int size(void);
	void mergeDetector(void);

public:
	virtual void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DetectorBase*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pVision;
	_DetectorBase* m_pDetIn;
	double m_overlapMin;
	OBJECT_DARRAY m_obj;

	string m_modelFile;
	string m_trainedFile;
	string m_meanFile;
	string m_labelFile;

	double m_sizeName;
	double m_sizeDist;
	Scalar m_colName;
	Scalar m_colDist;
	Scalar m_colObs;

	double m_contourBlend;
	bool m_bDrawContour;

};

}

#endif
