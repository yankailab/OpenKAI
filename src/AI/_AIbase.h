/*
 * _AIbase.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef AI__AIBASE_H_
#define AI__AIBASE_H_

#include "../Base/common.h"

#include "../Base/_ThreadBase.h"
#include "../Object/Object.h"
#include "../Stream/_StreamBase.h"

namespace kai
{

class _AIbase: public _ThreadBase
{
public:
	_AIbase();
	~_AIbase();

	virtual bool init(void* pKiss);
	virtual bool link(void);
	virtual bool start(void);
	virtual bool draw(void);

	virtual bool addObject(OBJECT* pNew);
	virtual Object* getObject(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_AIbase*) This)->update();
		return NULL;
	}

public:
	_StreamBase* m_pStream;
	Object* m_pObj;

	uint64_t m_frameID;

	string m_fileModel;
	string m_fileTrained;
	string m_fileMean;
	string m_fileLabel;

};

}

#endif
