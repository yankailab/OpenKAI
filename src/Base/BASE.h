/*
 * BASE.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_Base_BASE_H_
#define OpenKAI_src_Base_BASE_H_

#include "common.h"

using namespace std;

namespace kai
{
	class BASE
	{
	public:
		BASE();
		virtual ~BASE();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);

		virtual void pause(void);
		virtual void resume(void);
		virtual void stop(void);

		virtual void draw(void *pFrame);
		virtual void console(void *pConsole);
		virtual void context(void *pContext);

		virtual int serialize(uint8_t *pB, int nB);
		virtual int deSerialize(uint8_t *pB, int nB);

		string getName(void);
		string getClass(void);

	protected:
		void *m_pKiss;
		bool m_bLog;
	};

}

#endif
