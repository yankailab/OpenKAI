/*
 * _ObjectArray.h
 *
 *  Created on: June 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe__ObjectArray_H_
#define OpenKAI_src_Universe__ObjectArray_H_

#include "_Object.h"

namespace kai
{

	class _ObjectArray : public _ModuleBase
	{
	public:
		_ObjectArray();
		virtual ~_ObjectArray();

		virtual int init(void *pKiss);
		virtual int start(void);
		virtual void update(void);

		virtual int init(int n);

		//io
		virtual _Object *add(_Object &o);
		virtual _Object *get(int i);
		virtual void clear(void);
		virtual int size(void);

	private:
		static void *getUpdate(void *This)
		{
			((_ObjectArray *)This)->update();
			return NULL;
		}

	private:
		_Object *m_pO;
		int m_nBuf;
		int m_nO;
	};

}
#endif
