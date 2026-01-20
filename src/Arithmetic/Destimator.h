/*
 * Destimator.h
 *
 *  Created on: Mar 22, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Arithmetic_Destimator_H_
#define OpenKAI_src_Arithmetic_Destimator_H_

#include "../Base/BASE.h"
#include "PolyFit.h"

namespace kai
{

	class Destimator : public BASE
	{
	public:
		Destimator();
		virtual ~Destimator();

		virtual bool init(const json &j);
		virtual double v(int x);

	protected:
		PolyFit m_fit;
	};

}
#endif
