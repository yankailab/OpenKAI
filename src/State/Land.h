/*
 * Land.h
 *
 *  Created on: Dec 18, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_State_Land_H_
#define OpenKAI_src_State_Land_H_

#include "State.h"

namespace kai
{

	struct LAND_TAG
	{
		int m_iTag;
		float m_angle;

		void init(void)
		{
			m_iTag = 0;
			m_angle = 0.0;
		}
	};

	class Land : public State
	{
	public:
		Land();
		virtual ~Land();

		bool init(void *pKiss);
		bool update(void);

	public:
		int m_tag;
		float m_hdg;
		float m_speed;

		vector<LAND_TAG> m_vTag;
	};

}
#endif
