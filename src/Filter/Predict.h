/*
 * Predict.h
 *
 *  Created on: Mar 18, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Filter_Predict_H_
#define OpenKAI_src_Filter_Predict_H_

#include "FilterBase.h"

namespace kai
{

	template <class T>
	class Predict : public FilterBase<T>
	{
	public:
		Predict()
		{
			FilterBase<T>::m_nW = 2;
		}
		virtual ~Predict()
		{
		}

		bool init(T kT = 1)
		{
			m_kT = kT;
			FilterBase<T>::reset();
			return true;
		}

		using FilterBase<T>::update;

		T update(T v, T dT)
		{
			FilterBase<T>::add(v);

			if (FilterBase<T>::m_vV.size() < static_cast<size_t>(FilterBase<T>::m_nW))
			{
				FilterBase<T>::m_v = v;
				return FilterBase<T>::m_v;
			}

			size_t s = FilterBase<T>::m_vV.size();
			T p = FilterBase<T>::m_vV.at(s - 2);
			T q = FilterBase<T>::m_vV.at(s - 1);

			FilterBase<T>::m_v = q + (q - p) * dT * m_kT;
			return FilterBase<T>::m_v;
		}

	protected:
		T m_kT = 1;
	};

}
#endif
