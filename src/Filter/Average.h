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
			FilterBase<T>::m_nW = nW;
			if (FilterBase<T>::m_nW < 2)
				FilterBase<T>::m_nW = 2;

			FilterBase<T>::reset();

			m_base = 1.0 / FilterBase<T>::m_nW;
			return true;
		}

		T* update(T* pV)
		{
			NULL_N(pV);

			if(!FilterBase<T>::add(*pV))
			{
				FilterBase<T>::m_v = *pV;
				return FilterBase<T>::m_pV;
			}

			T tot = 0.0;
			for (int i = 0; i < FilterBase<T>::m_nW; i++)
				tot += FilterBase<T>::m_qV.at(i);

			FilterBase<T>::m_v = tot * m_base;
			return FilterBase<T>::m_pV;
		}

	protected:
		T m_base;
	};

}
#endif
