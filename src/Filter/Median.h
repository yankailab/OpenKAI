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
		Median<T>()
		{
			FilterBase<T>::m_nW = 3;
			m_iMid = 1;
		}
		virtual ~Median<T>()
		{
		}

		bool init(int nW)
		{
			FilterBase<T>::m_nW = nW;
			if (FilterBase<T>::m_nW < 3)
				FilterBase<T>::m_nW = 3;

			m_iMid = FilterBase<T>::m_nW / 2;
			FilterBase<T>::reset();

			return true;
		}

		T input(T v)
		{
			if(!FilterBase<T>::add(v))
			{
				FilterBase<T>::m_v = v;
				return FilterBase<T>::m_v;
			}

			m_qSort = FilterBase<T>::m_qV;
			std::sort(m_qSort.begin(), m_qSort.end());
			FilterBase<T>::m_v = m_qSort.at(m_iMid);

			return FilterBase<T>::m_v;
		}

		void reset(void)
		{
			FilterBase<T>::reset();
			m_qSort.clear();
		}

	private:
		int m_iMid;
		std::deque<T> m_qSort;
	};

}
#endif
