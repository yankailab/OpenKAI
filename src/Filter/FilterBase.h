/*
 * FilterBase.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Filter_FilterBase_H_
#define OpenKAI_src_Filter_FilterBase_H_

#include "../Base/common.h"

namespace kai
{

	template <class T>
	class FilterBase
	{
	public:
		FilterBase()
		{
			m_nTraj = 2;
			reset();
		}

		virtual ~FilterBase()
		{
			
		}

		virtual bool init(int nTraj)
		{
			m_nTraj = (nTraj < 2) ? 2 : nTraj;

			reset();
			return true;
		}

		virtual void input(T v)
		{
			while (m_qTraj.size() >= m_nTraj)
			{
				m_qTraj.pop_front();
			}
			m_qTraj.push_back(m_v);
		}

		virtual T v(void)
		{
			return m_v;
		}

		virtual void reset(void)
		{
			m_v = 0.0;
			m_variance = 0.0;
			m_qData.clear();
			m_qTraj.clear();
		}

	public:
		T m_v;
		std::deque<T> m_qData;

		unsigned int m_nTraj;
		std::deque<T> m_qTraj;

		T m_variance;
	};

}
#endif
