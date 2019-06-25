/*
 * _ObjectFactory.h
 *
 *  Created on: June 25, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe__ObjectFactory_H_
#define OpenKAI_src_Universe__ObjectFactory_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "_Object.h"

namespace kai
{

class _ObjectFactory: public _ThreadBase
{
public:
	_ObjectFactory();
	virtual ~_ObjectFactory();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual void update(void);
	virtual bool draw(void);
	virtual bool console(int& iY);

	_Object* create(void);
	void updateObjects(void);

private:
	static void* getUpdateThread(void* This)
	{
		((_ObjectFactory *) This)->update();
		return NULL;
	}

protected:
	_Object* m_pO;
	int		m_nCap;
	int		m_nO;

};

}
#endif
