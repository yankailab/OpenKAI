/*
 * _ObjectBase.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base__ObjectBase_H_
#define OpenKAI_src_Base__ObjectBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Vision/_VisionBase.h"

#define OBJECT_N_OBJ 256
#define OBJECT_N_CLASS 256

namespace kai
{

struct OBJECT
{
	vDouble4	m_bb;
	double		m_dist;
	vDouble2	m_vOpt;
	vDouble2	m_vTrack;
	uint64_t	m_trackID;
	void*		m_pTracker;
	int	 		m_topClass;		//most probable class
	double		m_topProb;		//prob for the topClass
	uint64_t 	m_mClass;		//all candidate class mask
	int64_t 	m_tStamp;

	void init(void)
	{
		m_bb.init();
		m_dist = -1.0;
		m_vOpt.init();
		m_vTrack.init();
		m_trackID = 0;
		m_pTracker = NULL;
		m_tStamp = -1;
		resetClass();
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

	vInt4 iBBox(vInt2 cSize)
	{
		vInt4 iBB;
		iBB.x = m_bb.x * cSize.x;
		iBB.z = m_bb.z * cSize.x;
		iBB.y = m_bb.y * cSize.y;
		iBB.w = m_bb.w * cSize.y;

		return iBB;
	}

	void setBB(vInt4 iBB, vInt2 cSize)
	{
		double base;
		base = 1.0/cSize.x;
		m_bb.x = iBB.x * base;
		m_bb.z = iBB.z * base;
		base = 1.0/cSize.y;
		m_bb.y = iBB.y * base;
		m_bb.w = iBB.w * base;
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

		return &m_pObj[m_nObj-1];
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
	string  m_name;
	int		m_n;

	void init(void)
	{
		m_n = 0;
		m_name = "";
	}
};

class _ObjectBase: public _ThreadBase
{
public:
	_ObjectBase();
	virtual ~_ObjectBase();

	virtual bool init(void* pKiss);
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual int getClassIdx(string& className);
	virtual string getClassName(int iClass);

	OBJECT* add(OBJECT* pNewObj);
	OBJECT* at(int i);
	void merge(_ObjectBase* pO);
	void updateStatistics(void);
	int size(void);

public:
	//input
	_VisionBase* m_pVision;
	OBJECT_DARRAY m_obj;
	Frame m_BGR;

	//control
	uint64_t m_trackID;
	bool	 m_bTrack;

	//config
	double m_dMaxTrack;
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
	int	   m_nClass;
	vector<OBJECT_CLASS> m_vClass;

	//show
	std::bitset<64> m_bitSet;
	bool m_bDrawStatistics;
	vInt3 m_classLegendPos;
	bool m_bDrawSegment;
	double m_segmentBlend;
	double m_drawVscale;
	bool m_bDrawObjClass;
	bool m_bDrawObjVtrack;

};

}
#endif
