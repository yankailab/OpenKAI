/*
 * _GstOutput.h
 *
 *  Created on: May 24, 2022
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_GstOutput_H_
#define OpenKAI_src_UI_GstOutput_H_

#include "_UIbase.h"
#include "../Vision/Frame.h"

namespace kai
{
	class _GstOutput : public _UIbase
	{
	public:
		_GstOutput();
		virtual ~_GstOutput();

		virtual bool init(const json& j);
		virtual bool start(void);

	protected:
		void updateGst(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_GstOutput *)This)->update();
			return NULL;
		}

	private:
		Frame m_F;
		vInt2 m_vSize;

		string m_gstOutput;
		VideoWriter m_gst;
	};
}
#endif
