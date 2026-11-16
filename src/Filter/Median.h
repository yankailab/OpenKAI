/*
 * Median.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Filter_Median_H_
#define OpenKAI_src_Filter_Median_H_

#include "FilterBase.h"

namespace kai
{

	template <class T>
	class Median : public FilterBase<T>
	{
	public:
		Median()
		{
			FilterBase<T>::m_nW = 3;
		}
		virtual ~Median()
		{
		}

		bool init(int nW)
		{
			if(nW < 3)
				nW = 3;

			FilterBase<T>::m_nW = nW;
			m_iMid = FilterBase<T>::m_nW / 2;
			FilterBase<T>::reset();
			return true;
		}

		T update(T v)
		{
			FilterBase<T>::add(v);

			if (FilterBase<T>::m_vV.size() < static_cast<size_t>(FilterBase<T>::m_nW))
			{
				FilterBase<T>::m_v = v;
				return FilterBase<T>::m_v;
			}

			m_vSort = FilterBase<T>::m_vV;
			std::sort(m_vSort.begin(), m_vSort.end());
			FilterBase<T>::m_v = m_vSort.at(m_iMid);

			return FilterBase<T>::m_v;
		}

		void reset(void)
		{
			FilterBase<T>::reset();
			m_vSort.clear();
		}

	private:
		int m_iMid = 1;
		std::vector<T> m_vSort;
	};

}
#endif
