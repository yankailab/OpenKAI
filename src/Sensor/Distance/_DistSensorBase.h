/*
 * _DistSensorBase.h
 *
 *  Created on: Apr 13, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__DistSensorBase_H_
#define OpenKAI_src_Sensor__DistSensorBase_H_

#include "../../Base/_ModuleBase.h"
#include "../../UI/_Console.h"
#include "../../Filter/Median.h"
#include "../../Filter/Average.h"
#ifdef USE_OPENCV
#include "../../Vision/Frame.h"
#endif

#define MAX_DIST_SENSOR_DIV 720

namespace kai
{

	enum DIST_SENSOR_TYPE
	{
		ds_Unknown,
		ds_LeddarVu,
		ds_TOFsense,
		ds_BenewakeTF,
	};

	struct DIST_SENSOR_DIV
	{
		float m_d; // raw input
		float m_a; // amplitude
		Median<float> m_fMed;
		Average<float> m_fAvr;

		void init(int nAvr, int nMed)
		{
			m_fMed.init(nMed);
			m_fAvr.init(nAvr);
			m_d = -1.0;
			m_a = -1.0;
		}

		void input(float d, float a)
		{
			m_a = a;
			m_d = d;
			if (d >= 0.0)
			{
				m_fAvr.update(m_fMed.update(m_d));
				return;
			}

			reset();
		}

		float d(void)
		{
			return m_d;
		}

		float a(void)
		{
			return m_a;
		}

		float dAvr(void)
		{
			if (m_d < 0.0)
				return -1.0;
			return m_fAvr.get();
		}

		float dMed(void)
		{
			if (m_d < 0.0)
				return -1.0;
			return m_fMed.get();
		}

		void reset(void)
		{
			m_fMed.reset();
			m_fAvr.reset();
		}
	};

	class _DistSensorBase : public _ModuleBase
	{
	public:
		_DistSensorBase();
		virtual ~_DistSensorBase();

		virtual bool init(const json &j);
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

		bool bReady(void);
		vFloat2 range(void);
		void input(float deg, float d, float a);
		void input(int iDiv, float d, float a);
		void input(float d, float a = -1);

		virtual DIST_SENSOR_TYPE type(void);
		virtual float d(int iDiv = 0);
		virtual float d(float deg = 0);
		virtual float dMin(void);
		virtual float dMax(void);
		virtual float dAvr(void);
		virtual float dMin(float degFrom, float degTo);
		virtual float dMax(float degFrom, float degTo);
		virtual float dAvr(float degFrom, float degTo);

	protected:
		DIST_SENSOR_DIV *m_pDiv;
		int m_nDiv;

		float m_fovH;
		float m_fovV;

		float m_dDeg;
		float m_dDegInv;

		vFloat2 m_vRange;
		float m_calibScale;
		float m_calibOffset;

		uint16_t m_bReady;
	};

}
#endif
