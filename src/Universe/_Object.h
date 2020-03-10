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

	//size
	void setDim(vFloat3 s);
	void setW(float w);
	void setH(float h);
	void setD(float d);

	vFloat3 getDim(void);
	float getW(void);
	float getH(void);
	float getD(void);

	float area(void);
	float volume(void);

	//classification
	void resetClass(void);
	void addClassIdx(int iClass);
	void setClassMask(uint64_t mClass);
	void setTopClass(int iClass, float prob);
	bool bClass(int iClass);
	bool bClassMask(uint64_t mClass);

	//interactions
	virtual float nIoU(_Object* pO);

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
	vFloat3		m_vPos;		//position, x,y,z
	vFloat3		m_vAtti;	//attitude
	vFloat3		m_vDim;		//width, height, depth

	//kinetics
	vFloat3		m_vSpeed;
	vFloat3		m_vAccel;

	//classification
	string		m_txt;
	int			m_topClass;		//most probable class
	float		m_topProb;		//prob for the topClass
	uint64_t	m_mClass;		//all candidate class mask

	//Trajectory
	vector<vFloat3> m_vTraj;

	//bitflag
	uint64_t	m_mFlag;

	//Tracker
	void *m_pTracker;

};

}
#endif
