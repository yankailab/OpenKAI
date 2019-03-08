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

namespace kai
{

struct OBJ_BB2
{
	vFloat4		m_bb;

	void setBB(Rect& r, vInt2& cs)
	{
		float b;
		b = 1.0 / cs.x;
		m_bb.x = r.x * b;
		m_bb.z = (r.x + r.width) * b;
		b = 1.0 / cs.y;
		m_bb.y = r.y * b;
		m_bb.w = (r.y + r.height) * b;
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
};

struct OBJ_BB4
{
	vFloat4		m_bb;
	vFloat2		m_pP[4];

	Rect getRect(void)
	{
		vector<Point> vP;
		Point p;
		for(int i=0;i<4;i++)
		{
			p.x = m_pP[i].x;
			p.y = m_pP[i].y;
			vP.push_back(p);
		}
		return boundingRect(vP);
	}

	vFloat4 getBB(vInt2& cs)
	{
		Rect r = getRect();

		float b;
		b = 1.0 / cs.x;
		m_bb.x = r.x * b;
		m_bb.z = (r.x + r.width) * b;
		b = 1.0 / cs.y;
		m_bb.y = r.y * b;
		m_bb.w = (r.y + r.height) * b;

		return m_bb;
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
};

struct OBJ_MARKER
{
	vFloat2		m_p;
	float		m_r;
	float		m_angle;
};

#ifdef USE_OCR
#define OBJ_TEXT_N_BUF 16
struct OBJ_TEXT
{
	vFloat4		m_bb;
	vFloat2		m_pP[4];
	char		m_pText[OBJ_TEXT_N_BUF];

	Rect getRect(void)
	{
		vector<Point> vP;
		Point p;
		for(int i=0;i<4;i++)
		{
			p.x = m_pP[i].x;
			p.y = m_pP[i].y;
			vP.push_back(p);
		}
		return boundingRect(vP);
	}

	vFloat4 getBB(vInt2& cs)
	{
		Rect r = getRect();

		float b;
		b = 1.0 / cs.x;
		m_bb.x = r.x * b;
		m_bb.z = (r.x + r.width) * b;
		b = 1.0 / cs.y;
		m_bb.y = r.y * b;
		m_bb.w = (r.y + r.height) * b;

		return m_bb;
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
};
#endif

union UNI_OBJECT
{
	OBJ_BB2 m_bb2;
	OBJ_BB4 m_bb4;
	OBJ_MARKER m_marker;

#ifdef USE_OCR
	OBJ_TEXT m_text;
#endif
};

enum OBJ_TYPE
{
	obj_unknown,
	obj_bb2,
	obj_bb4,
	obj_marker,

#ifdef USE_OCR
	obj_text,
#endif
};

struct OBJECT
{
	UNI_OBJECT	m_o;
	OBJ_TYPE	m_type;
	int			m_topClass;		//most probable class
	float		m_topProb;		//prob for the topClass
	uint64_t	m_mClass;		//all candidate class mask
	void*		m_pTracker;
	int64_t		m_tStamp;

	void init(void)
	{
		m_type = obj_unknown;
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

	float area(void)
	{
		switch (m_type)
		{
		case obj_bb2:
			return m_o.m_bb2.m_bb.area();
		case obj_bb4:
			return m_o.m_bb4.m_bb.area();
		default:
			return -1;
		}
	}

	float width(void)
	{
		switch (m_type)
		{
		case obj_bb2:
			return m_o.m_bb2.m_bb.width();
		case obj_bb4:
			return m_o.m_bb4.m_bb.width();
		default:
			return -1;
		}
	}

	float height(void)
	{
		switch (m_type)
		{
		case obj_bb2:
			return m_o.m_bb2.m_bb.height();
		case obj_bb4:
			return m_o.m_bb4.m_bb.height();
		default:
			return -1;
		}
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
	Frame m_BGR;
	OBJECT_DARRAY m_obj;

	//config
	double m_minConfidence;
	double m_minArea;
	double m_maxArea;
	double m_maxW;
	double m_maxH;
	vDouble4 m_roi;

	//model
	string m_modelFile;
	string m_trainedFile;
	string m_meanFile;
	string m_labelFile;
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
