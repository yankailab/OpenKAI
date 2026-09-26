/*
 * _D2RGB.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_Pipeline__D2RGB_H_
#define OpenKAI_src_Vision_Pipeline__D2RGB_H_

#include "../RGBD/_RGBDbase.h"
#include <mutex>

namespace kai
{

	class _D2RGB : public _RGBDbase
	{
	public:
		_D2RGB();
		virtual ~_D2RGB();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual void draw(void *pMat);

		virtual float d(const Vector4i &bb);

	private:
		void filter(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_D2RGB *)This)->update();
			return NULL;
		}

	protected:
		_RGBDbase *m_pVd = nullptr;

		int m_nHistLev = 128;
		int m_iHistFrom = 0;
		float m_minHistD = 0.25;
		bool m_bMeasure = true;

	};

}
#endif
