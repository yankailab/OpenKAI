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
			m_wLen = 3;
			m_iMid = 1;
			FilterBase<T>::reset();
		}
		virtual ~Median<T>()
		{
			
		}

		bool init(int wLen, int nTraj)
		{
			IF_F(!FilterBase<T>::init(nTraj));

			m_wLen = wLen;
			if (m_wLen < 3)
				m_wLen = 3;

			m_iMid = m_wLen / 2;
			FilterBase<T>::reset();

			return true;
		}

		void input(T v)
		{
			if (std::isnan(v))
				v = 0;
			FilterBase<T>::m_qData.push_back(v);
			if (FilterBase<T>::m_qData.size() < m_wLen)
			{
				FilterBase<T>::m_v = v;
				FilterBase<T>::input(v);
				return;
			}

			while (FilterBase<T>::m_qData.size() > m_wLen)
			{
				FilterBase<T>::m_qData.pop_front();
			}

			m_qSort = FilterBase<T>::m_qData;
			std::sort(m_qSort.begin(), m_qSort.end());
			FilterBase<T>::m_v = m_qSort.at(m_iMid);

			FilterBase<T>::input(v);
		}
		void reset(void)
		{
			FilterBase<T>::reset();
			m_qSort.clear();
		}

	private:
		int m_wLen;
		int m_iMid;
		std::deque<T> m_qSort;
	};

}
#endif
