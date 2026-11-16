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
		Average()
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
			for (const auto &value : FilterBase<T>::m_vV)
				tot += value;

			FilterBase<T>::m_v = tot / (T)FilterBase<T>::m_vV.size();
			return FilterBase<T>::m_v;
		}
	};

}
#endif
