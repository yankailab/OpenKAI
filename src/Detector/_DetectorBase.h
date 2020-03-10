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
#include "../Vision/_DepthVisionBase.h"
#include "../Protocol/_Mavlink.h"


#define OBJECT_N_OBJ 256
#define OBJ_N_CHAR 32
#define OBJ_N_VERTICES 32
#define OBJ_N_TRAJ 16

namespace kai
{

struct OBJECT
{
	//BBox normalized to 0.0 to 1.0
	vFloat4 m_bb;
	float m_dist;

	//Vertices in pixel unit
	vFloat2 m_pV[OBJ_N_VERTICES];
	int m_nV;

	//Center position and radius/angle
	vFloat2 m_c;
	float m_r;
	float m_angle;

	//Text
	char m_pText[OBJ_N_CHAR];

	//Classification
	int m_topClass;		//most probable class
	float m_topProb;		//prob for the topClass
	uint64_t m_mClass;		//all candidate class mask

	//Tracker
	void *m_pTracker;

	//Properties
	uint64_t m_tStamp;

	//Trajectory
	vFloat2 m_pTraj[OBJ_N_TRAJ];
	int m_iTraj;
	int m_nTraj;

	void init(void)
	{
		m_bb.init();
		m_dist = -1.0;
		m_nV = 0;
		m_iTraj = 0;
		m_nTraj = 0;

		resetClass();
		m_pTracker = NULL;
		m_tStamp = 0;
	}

	void addClassIdx(int iClass)
	{
		m_mClass |= 1 << iClass;
	}

	void setClassMask(uint64_t mClass)
	{
		m_mClass = mClass;
	}

	void setTopClass(int iClass, float prob)
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

	void setBB(vFloat4 bb)
	{
		m_bb = bb;
	}

	void setVertices(vFloat2 *pV, int nV)
	{
		NULL_(pV);
		int i;

		vector<Point> vP;
		Point p;
		m_nV = nV;
		for (i = 0; i < m_nV; i++)
		{
			m_pV[i] = pV[i];
			p.x = m_pV[i].x;
			p.y = m_pV[i].y;
			vP.push_back(p);
		}

		m_bb = convertBB<vFloat4>(boundingRect(vP));
	}

	void normalizeBB(vInt2 vB)
	{
		float b;
		b = 1.0 / vB.x;
		m_bb.x *= b;
		m_bb.z *= b;
		b = 1.0 / vB.y;
		m_bb.y *= b;
		m_bb.w *= b;

		m_bb.constrain(0.0, 1.0);
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

	void addTrajectory(const vFloat2 &p)
	{
		if (m_nTraj == 0)
		{
			m_pTraj[m_iTraj++] = p;
			m_nTraj++;
			return;
		}

		int iLast = m_iTraj - 1;
		if (iLast < 0)
			iLast = m_nTraj - 1;
		vFloat2 lp = m_pTraj[iLast];
		float d = abs(lp.x - p.x) + abs(lp.y - p.y);
		IF_(d < 0.02);

		m_pTraj[m_iTraj++] = p;
		if (m_iTraj >= OBJ_N_TRAJ)
			m_iTraj = 0;
		if (m_nTraj < OBJ_N_TRAJ)
			m_nTraj++;
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

	OBJECT* add(OBJECT *pO)
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

	virtual bool init(void *pKiss);
	virtual void draw(void);
	virtual int getClassIdx(string &className);
	virtual string getClassName(int iClass);

	//io
	virtual OBJECT* add(OBJECT *pNewObj);
	virtual OBJECT* at(int i);
	virtual void resetObj(void);
	virtual void updateObj(void);

	//pipeline
	virtual void pipeIn(void);

	void updateStatistics(void);
	int size(void);

public:
	//input
	_VisionBase *m_pV;
	_DepthVisionBase* m_pDV;
	_DetectorBase *m_pDB;

	//data
	Frame m_fBGR;
	int m_iSwitch;
	OBJECT_ARRAY m_pObj[2];
	OBJECT_ARRAY *m_pPrev;
	OBJECT_ARRAY *m_pNext;

	//config
	float m_minConfidence;
	float m_minArea;
	float m_maxArea;
	float m_minW;
	float m_maxW;
	float m_minH;
	float m_maxH;
	bool m_bMerge;
	float m_mergeOverlap;
	float m_bbScale;
	vFloat4 m_vRoi;

	//model
	string m_modelFile;
	string m_trainedFile;
	string m_meanFile;
	string m_classFile;
	int m_nClass;
	vector<OBJECT_CLASS> m_vClass;

	//show
	bool m_bDrawStatistics;
	vInt3 m_classLegendPos;
	bool m_bDrawClass;
	bool m_bDrawText;
	bool m_bDrawPos;

};

}
#endif
