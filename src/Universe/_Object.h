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
	virtual bool draw(void);
	virtual bool console(int& iY);

	//size
	void setSize(vFloat3 s);
	void setLength(float l);
	void setWidth(float w);
	void setHeight(float h);
	void setRadius(float r);

	vFloat3 getSize(void);
	float getLength(void);
	float getWidth(void);
	float getHeight(void);
	float getRadius(void);
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
	virtual bool IoU(_Object* pO);

	//kinetics
	virtual void updateKinetics(void);

private:
	static void* getUpdateThread(void* This)
	{
		((_Object *) This)->update();
		return NULL;
	}

protected:
	//pos and attitude
	vFloat3		m_p;		//position
	vFloat3		m_a;		//attitude
	vFloat4		m_q;		//quaternion

	//kinetics
	vFloat3		m_speed;
	vFloat3		m_accel;

	//size
	vFloat4		m_bb;	//bbox:x(length),y(width),z(height), bball radius:r

	//classification
	string		m_txt;
	int			m_topClass;		//most probable class
	float		m_topProb;		//prob for the topClass
	uint64_t	m_mClass;		//all candidate class mask

};

}
#endif
