/*
 * Average.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Filter_Average_H_
#define OpenKAI_src_Filter_Average_H_

#include "FilterBase.h"

namespace kai
{

	template <class T>
	class Average : public FilterBase<T>
	{
	public:
		Average<T>()
		{
			m_wLen = 1;
			FilterBase<T>::reset();
		}
		virtual ~Average()
		{
			
		}

		bool init(int wLen, int nTraj)
		{
			IF_F(!FilterBase<T>::init(nTraj));

			m_wLen = wLen;
			if (m_wLen < 1)
				m_wLen = 1;

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

			T tot = 0.0;
			for (int i = 0; i < m_wLen; i++)
				tot += FilterBase<T>::m_qData.at(i);

			FilterBase<T>::m_v = tot / (T)m_wLen;
			FilterBase<T>::input(v); //trajctory
		}

	private:
		int m_wLen;
	};

}
#endif
