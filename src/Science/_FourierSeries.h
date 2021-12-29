/*
 * ScienceBase.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Science__FourierSeries_H_
#define OpenKAI_src_Science__FourierSeries_H_

#include "_ScienceBase.h"

namespace kai
{

	class _FourierSeries : public _ScienceBase
	{
	public:
		_FourierSeries();
		virtual ~_FourierSeries();

		virtual bool init(void *pKiss);
		virtual bool start(void);

	protected:
		static int cbGraph(mglGraph *gr);
		virtual void updateSeries(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_FourierSeries *)This)->update();
			return NULL;
		}

	private:
		int m_S;
		int m_M;
		int m_N;
		double m_w;

		vDouble2 m_vOrigin;
		vDouble2 m_vRangeX;
		vDouble2 m_vRangeY;

		string m_fy;

		int m_nDigit;

	};

}
#endif
