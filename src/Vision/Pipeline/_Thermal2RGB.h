/*
 * _Thermal2RGB.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Thermal2RGB_H_
#define OpenKAI_src_Vision__Thermal2RGB_H_

#include "../_VisionBase.h"

namespace kai
{

	class _Thermal2RGB : public _VisionBase
	{
	public:
		_Thermal2RGB();
		virtual ~_Thermal2RGB();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual void console(const json &j, void *pJSONbase);

	private:
		void filter(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Thermal2RGB *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV = nullptr;

		Vector2f m_vTrange = Vector2f::Zero();	// temperature range to show

	};

}
#endif
