/*
 * _Remap.h
 *
 *  Created on: May 7, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Remap_H_
#define OpenKAI_src_Vision__Remap_H_

#include "../_VisionBase.h"

namespace kai
{

	class _Remap : public _VisionBase
	{
	public:
		_Remap();
		virtual ~_Remap();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);

		bool setCamMat(const Mat &mC, const Mat &mD);
		bool scaleCamMat(void);
		//	void updateCamMat(void);
		vDouble2 getF(void);
		vDouble2 getC(void);
		vFloat2 getFf(void);
		vFloat2 getCf(void);
		Mat mC(void);
		Mat mCscaled(void);
		Mat mD(void);

	private:
		void filter(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Remap *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV;
		bool m_bReady;
		string m_fCalib;

		// original camera matrix
		Mat m_mC;		// Intrinsic
		Mat m_mCscaled; // scaled with input image size
		Mat m_mD;		// Distortion
		// Remap
		Mat m_m1;
		Mat m_m2;
	};

}
#endif
