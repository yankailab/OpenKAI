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
		Predict<T>()
		{
			FilterBase<T>::m_nW = 2;
		}
		virtual ~Predict()
		{
		}

		bool init(float kT)
		{
			m_kT = kT;
			FilterBase<T>::reset();
			return true;
		}

		using FilterBase<T>::update;

		T update(T v, float dT)
		{
			FilterBase<T>::add(v);

			if (FilterBase<T>::m_qV.size() < static_cast<size_t>(FilterBase<T>::m_nW))
			{
				FilterBase<T>::m_v = v;
				return FilterBase<T>::m_v;
			}

			size_t s = FilterBase<T>::m_qV.size();
			T p = FilterBase<T>::m_qV.at(s - 2);
			T q = FilterBase<T>::m_qV.at(s - 1);

			FilterBase<T>::m_v = q + (q - p) * (T)dT * m_kT;
			return FilterBase<T>::m_v;
		}

	protected:
		float m_kT = 1.0;
	};

}
#endif
