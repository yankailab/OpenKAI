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

		bool init(int nW)
		{
			IF_F(nW < 0);
			FilterBase<T>::m_nW = nW;
			reset();
			return true;
		}

		T *update(T *pV, float dT)
		{
			NULL_N(pV);
			if (FilterBase<T>::m_nW < 2 ||
				FilterBase<T>::m_qV.size() < FilterBase<T>::m_nW)
			{
				FilterBase<T>::m_v = *pV;
				return FilterBase<T>::m_pV;
			}

			FilterBase<T>::add(*pV);
			int s = FilterBase<T>::m_qV.size();
			T p = FilterBase<T>::m_qV.at(s - 2);
			T q = FilterBase<T>::m_qV.at(s - 1);
				
			FilterBase<T>::m_v = q + (q - p) * (T)dT;
			return FilterBase<T>::m_pV;
		}

		void reset(void)
		{
			FilterBase<T>::reset();
		}
	};

}
#endif
