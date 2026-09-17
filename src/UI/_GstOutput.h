/*
 * _GstOutput.h
 *
 *  Created on: May 24, 2022
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_GstOutput_H_
#define OpenKAI_src_UI_GstOutput_H_

#include "_UIbase.h"
#include "../Base/cv.h"

namespace kai
{
	class _GstOutput : public _UIbase
	{
	public:
		_GstOutput();
		virtual ~_GstOutput();

		virtual bool init(const json& j);
		virtual bool start(void);

	private:
		void updateGst(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_GstOutput *)This)->update();
			return NULL;
		}

	protected:
		Mat m_M;
		Vector2i m_vSize = Vector2i::Zero();

		string m_gstOutput = "appsrc ! videoconvert ! fbdevsink";
		VideoWriter m_gst;
	};
}
#endif
