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
			FilterBase<T>::m_nW = 2;
		}
		virtual ~Average()
		{
		}

		bool init(int nW)
		{
			if(nW < 2)
				nW = 2;

			FilterBase<T>::m_nW = nW;
			FilterBase<T>::reset();
			return true;
		}

		T update(T v)
		{
			FilterBase<T>::add(v);
			T tot = 0.0;
			int n = FilterBase<T>::m_qV.size();
			for (int i = 0; i < n; i++)
				tot += FilterBase<T>::m_qV.at(i);

			FilterBase<T>::m_v = tot / (T)n;
			return FilterBase<T>::m_v;
		}
	};

}
#endif
