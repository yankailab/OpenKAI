/*
 * _DetectorBase.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base__DetectorBase_H_
#define OpenKAI_src_Base__DetectorBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Vision/_VisionBase.h"

#define OBJECT_N_OBJ 256
#define OBJECT_N_CLASS 256
#define OBJ_N_CHAR 32
#define OBJ_N_VERTICES 32

namespace kai
{

struct OBJECT
{
	//BBox normalized to 0.0 to 1.0
	vFloat4		m_bb;
	float		m_dist;

	//Vertices in pixel unit
	vFloat2		m_pV[OBJ_N_VERTICES];
	int			m_nV;

	//Center position and radius/angle
	vFloat2		m_c;
	float		m_r;
	float		m_angle;

	//Text
	char		m_pText[OBJ_N_CHAR];

	//Classification
	int			m_topClass;		//most probable class
	float		m_topProb;		//prob for the topClass
	uint64_t	m_mClass;		//all candidate class mask

	//Tracker
	void*		m_pTracker;

	//Properties
	int64_t		m_tStamp;

	void init(void)
	{
		m_bb.init();
		m_dist = 0.0;
		m_nV = 0;

		resetClass();
		m_pTracker = NULL;
		m_tStamp = -1;
	}

	void addClassIdx(int iClass)
	{
		m_mClass |= 1 << iClass;
	}

	void setClassMask(uint64_t mClass)
	{
		m_mClass = mClass;
	}

	void setTopClass(int iClass, double prob)
	{
		m_topClass = iClass;
		m_topProb = prob;
		addClassIdx(iClass);
	}

	bool bClass(int iClass)
	{
		return (m_mClass & (1 << iClass));
	}

	bool bClassMask(uint64_t mClass)
	{
		return m_mClass & mClass;
	}

	void resetClass(void)
	{
		m_topClass = -1;
		m_topProb = 0.0;
		m_mClass = 0;
	}

	void setBB(Rect& r, vInt2& cs)
	{
		float b;
		b = 1.0 / cs.x;
		m_bb.x = r.x * b;
		m_bb.z = (r.x + r.width) * b;
		b = 1.0 / cs.y;
		m_bb.y = r.y * b;
		m_bb.w = (r.y + r.height) * b;

		m_bb.constrain(0.0,1.0);
	}

	Rect getBoundingRect(void)
	{
		vector<Point> vP;
		Point p;
		for(int i=0; i<m_nV; i++)
		{
			p.x = m_pV[i].x;
			p.y = m_pV[i].y;
			vP.push_back(p);
		}
		return boundingRect(vP);
	}

	void updateBB(vInt2& cs)
	{
		Rect r = getBoundingRect();
		setBB(r,cs);
	}

	Rect getRect(vInt2& cs)
	{
		Rect r;
		r.x = m_bb.x * cs.x;
		r.y = m_bb.y * cs.y;
		r.width = m_bb.z * cs.x - r.x;
		r.height = m_bb.w * cs.y - r.y;
		return r;
	}

	float area(void)
	{
		return m_bb.area();
	}

	float width(void)
	{
		return m_bb.width();
	}

	float height(void)
	{
		return m_bb.height();
	}
};

struct OBJECT_ARRAY
{
	OBJECT m_pObj[OBJECT_N_OBJ];
	int m_nObj;

	void reset(void)
	{
		m_nObj = 0;
	}

	OBJECT* add(OBJECT* pO)
	{
		NULL_N(pO);
		IF_N(m_nObj >= OBJECT_N_OBJ);

		m_pObj[m_nObj++] = *pO;

		return &m_pObj[m_nObj - 1];
	}

	OBJECT* at(int i)
	{
		IF_N(i >= m_nObj);
		IF_N(i < 0);
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

struct OBJECT_CLASS
{
	string m_name;
	int m_n;

	void init(void)
	{
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
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual int getClassIdx(string& className);
	virtual string getClassName(int iClass);

	OBJECT* add(OBJECT* pNewObj);
	OBJECT* at(int i);
	void merge(_DetectorBase* pO);
	void updateStatistics(void);
	int size(void);

public:
	//input
	_VisionBase* m_pVision;
	Frame m_fBGR;
	OBJECT_DARRAY m_obj;

	//config
	double m_minConfidence;
	double m_minArea;
	double m_maxArea;
	double m_minW;
	double m_maxW;
	double m_minH;
	double m_maxH;
	bool   m_bMerge;
	float	m_mergeOverlap;
	float	m_bbScale;

	//model
	string m_modelFile;
	string m_trainedFile;
	string m_meanFile;
	string m_classFile;
	int m_nClass;
	vector<OBJECT_CLASS> m_vClass;

	//show
	std::bitset<64> m_bitSet;
	bool m_bDrawStatistics;
	vInt3 m_classLegendPos;
	bool m_bDrawSegment;
	double m_segmentBlend;
	double m_drawVscale;
	bool m_bDrawObjClass;

};

}
#endif
