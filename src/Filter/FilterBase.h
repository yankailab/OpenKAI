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
		}

		virtual ~FilterBase()
		{	
		}

		virtual T update(T v)
		{
		}

		virtual void add(T v)
		{
			m_qV.push_back(v);
			while (m_qV.size() > m_nW)
				m_qV.pop_front();
		}

		virtual void reset(void)
		{
			m_v = 0;
			m_qV.clear();
			m_variance = 0;
		}

		virtual T get(void)
		{
			return m_v;
		}

	protected:
		T m_v;				//output
		std::deque<T> m_qV;	//trajectory
		int m_nW;			//window length
		T m_variance;
	};

}
#endif
