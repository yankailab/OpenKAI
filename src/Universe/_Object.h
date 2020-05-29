/*
 * _Object.h
 *
 *  Created on: June 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe__Object_H_
#define OpenKAI_src_Universe__Object_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"

namespace kai
{

class _Object: public _ThreadBase
{
public:
	_Object();
	virtual ~_Object();

	//general
	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual void update(void);
	virtual void draw(void);

	//pos
	void setPos(vFloat3& p);
	vFloat3 getPos(void);

	//attitude
	void setAttitude(vFloat3& a);
	vFloat3 getAttitude(void);

	//dimension
	void setDim(vFloat4& d);
	void setW(float w);
	void setH(float h);
	void setD(float d);
	void setR(float r);

	vFloat4 getDim(void);
	float getW(void);
	float getH(void);
	float getD(void);
	float getR(void);

	float area(void);
	float volume(void);

	//convenient
	void setBB2D(vFloat4& bb);
	vFloat4 getBB2D(void);
	vFloat4 getBB2Dnorm(float nx, float ny);

	//vertex
	void setVertices2D(vFloat2 *pV, int nV);

	//classification
	void resetClass(void);
	void addClassIdx(int iClass);
	void setClassMask(uint64_t mClass);
	void setTopClass(int iClass, float prob);
	bool bClass(int iClass);
	bool bClassMask(uint64_t mClass);

	//interactions
	virtual float nIoU(_Object& obj);

	//kinetics
	virtual void updateKinetics(void);

private:
	static void* getUpdateThread(void* This)
	{
		((_Object *) This)->update();
		return NULL;
	}

protected:
	//general
	vFloat3		m_vPos;		//center pos x,y,z
	vFloat3		m_vAtti;	//attitude
	vFloat4		m_vDim;		//width, height, depth, radius

	//kinetics
	vFloat3		m_vSpeed;
	vFloat3		m_vAccel;

	//classification
	int			m_topClass;		//most probable class
	float		m_topProb;		//prob for the topClass
	uint64_t	m_mClass;		//all candidate class mask
	string		m_txt;

	//trajectory
	vector<vFloat3> m_vTraj;

	//vertex
	vector<vFloat2> m_vVertex;


	//bitflag
	uint64_t	m_mFlag;

	//Tracker
	void *m_pTracker;

};

}
#endif
