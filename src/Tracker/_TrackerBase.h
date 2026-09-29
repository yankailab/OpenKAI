/*
 * _TrackerBase.h
 *
 *  Created on: Aug 28, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Tracker__TrackerBase_H_
#define OpenKAI_src_Tracker__TrackerBase_H_

#include <opencv2/tracking.hpp>
#include "../Base/_ModuleBase.h"
#include "../Utility/utilCV.h"
#include "../DataObject/RGBframe.h"
#include "../DataObject/BBoxStream.h"

namespace kai
{

	enum TRACK_STATE
	{
		track_init,
		track_update,
		track_stop
	};

	class _TrackerBase : public _ModuleBase
	{
	public:
		_TrackerBase();
		virtual ~_TrackerBase();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual void update(void);
		virtual bool check(void);
		virtual void draw(void *pMat);
		virtual void console(void *pConsole);

		virtual void createTracker(void);
		virtual bool startTrack(const Vector4f &bb);
		virtual void stopTrack(void);

	protected:
		RGBframe *m_pRGBin = nullptr;
		BBoxStream *m_pBBout = nullptr;
		std::mutex m_mutex;
		uint64_t m_tFrame = 0;
		uint64_t m_tPublished = 0;
		Rect m_rBB;
		Vector4f m_bb = Vector4f::Zero();
		float m_margin = 0.0;

		Rect m_newBB;
		uint64_t m_iSet = 0;
		uint64_t m_iInit = 0;

		string m_trackerType = "";
		TRACK_STATE m_trackState = track_stop;
	};

}
#endif
