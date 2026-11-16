/*
 * FilterBase.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Filter_FilterBase_H_
#define OpenKAI_src_Filter_FilterBase_H_

#include "../Base/common.h"
#include <vector>

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
			m_v = v;
			return m_v;
		}

		virtual void add(T v)
		{
			m_vV.push_back(v);
			if (m_nW <= 0)
				return;

			if (m_vV.size() > static_cast<size_t>(m_nW))
				m_vV.erase(m_vV.begin(), m_vV.end() - m_nW);
		}

		virtual void reset(void)
		{
			m_v = 0;
			m_vV.clear();
			m_variance = 0;
		}

		virtual T get(void)
		{
			return m_v;
		}

	protected:
		T m_v;				//output
		std::vector<T> m_vV;	//trajectory
		int m_nW = 0;		//window length
		T m_variance;
	};

}
#endif
