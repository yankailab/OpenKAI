/*
 * Loiter.h
 *
 *  Created on: Jan 14, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_State_Loiter_H_
#define OpenKAI_src_State_Loiter_H_

#include "State.h"

namespace kai
{

	class Loiter : public State
	{
	public:
		Loiter();
		virtual ~Loiter();

		bool init(void *pKiss);
		bool update(void);
		void reset(void);
		void console(void *pConsole);

		void setPos(vDouble3 &p);

	private:
		vDouble3 m_vPos; //lat, lon, alt
		double m_hdg;
	};

}
#endif
