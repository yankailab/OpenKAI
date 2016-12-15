/*
 * _Universe.h
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#ifndef SRC_NAVIGATION__UNIVERSE_H_
#define SRC_NAVIGATION__UNIVERSE_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Stream/Frame.h"

#define NUM_OBJ 128
#define NUM_OBJ_CLASS 128

namespace kai
{

struct OBJ_CLASS
{
	string	m_name;
	uint8_t	m_safety;
};

struct OBJECT
{
	uint64_t	m_frameID;	//0:vacant
	vInt4 		m_bbox;
	double 		m_dist;
	double		m_prob;
	vInt2		m_camSize;
	int			m_iClass;
};

class _Universe: public _ThreadBase
{
public:
	_Universe();
	virtual ~_Universe();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	void reset(void);

	int addObjClass(string* pName, uint8_t safety);

	OBJECT* addObject(OBJECT* pNewObj);
	OBJECT* getObjectByClass(int iClass);

	bool draw(void);

private:
	void updateObject(void);
	void deleteObject(int i);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Universe*) This)->update();
		return NULL;
	}

public:
	uint64_t m_frameID;
	uint64_t m_frameLifeTime;
	int m_disparity;
	double m_objProbMin;

	OBJ_CLASS m_pObjClass[NUM_OBJ_CLASS];
	int m_nObjClass;

	OBJECT m_pObj[NUM_OBJ];
	int m_nObj;


};

} /* namespace kai */

#endif /* SRC_NAVIGATION__UNIVERSE_H_ */
