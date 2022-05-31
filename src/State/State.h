/*
 * Mission.h
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Mission_Mission_H_
#define OpenKAI_src_Mission_Mission_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"

namespace kai
{

	enum STATE_TYPE
	{
		state_unknown,
		state_base,
		state_land,
		state_loiter,
		state_goto,
		state_rth,
		state_wp,
		state_takeoff,
	};

	class State : public BASE
	{
	public:
		State();
		virtual ~State();

		virtual bool init(void *pKiss);
		virtual bool update(void);
		virtual void reset(void);
		virtual STATE_TYPE type(void);

		virtual void complete(void);

	public:
		STATE_TYPE m_type;
		string m_next;

		uint64_t m_tStart;
		uint64_t m_tStamp;
		uint64_t m_tTimeout;
		bool m_bComplete;
	};

}
#endif
