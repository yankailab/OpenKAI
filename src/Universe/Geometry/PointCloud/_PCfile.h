/*
 * _PCfile.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud_PCfile_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_PCfile_H_

#include "../../_ReferenceFrame.h"
#include "../../../DataObject/PCLframe.h"

namespace kai
{

	class _PCfile : public _ReferenceFrame
	{
	public:
		_PCfile();
		virtual ~_PCfile();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool link(InstanceMgr *pM) override;
		bool check(void) override;
		virtual void clear(void);
		virtual bool start(void);
		bool open(void);
		// Binary little-endian XYZ float32 and RGB uint8. Nonfinite
		// color channels become white; finite channels are clamped to [0, 1].
		// The parent directory must exist. Replace the destination only on success.
		static bool savePLY(const string &path, const vector<GEOMETRY_POINT> &points,
			string *error = nullptr);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCfile *)This)->update();
			return NULL;
		}

	protected:
		PCLframe *m_pPCL = nullptr;
		vector<string> m_vfName;
	};

}
#endif
