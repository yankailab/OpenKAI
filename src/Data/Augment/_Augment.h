/*
 * _Augment.h
 *
 *  Created on: Oct 12, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__Augment_H_
#define OpenKAI_src_Data_Augment__Augment_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "../_DataBase.h"

#define N_CMD 128

namespace kai
{

class _Augment: public _DataBase
{
public:
	_Augment();
	~_Augment();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	void reset(void);

private:
	void rotate(void);
	void noise(void);
	void shrink(void);
	void lowResolution(void);
	void crop(void);
	void flip(void);
	void contrast(void);
	void brightness(void);
	void histEqualize(void);
	void blur(void);
	void gaussianBlur(void);
	void medianBlur(void);
	void bilateralBlur(void);

	void adaptHistEqualize(void);
	void move(void);
	void tone(void);
	void channelShift(void);
	void shearing(void);
	void vignetting(void);
	void decolor(void);
	void invColor(void);
	void posterize(void);
	void erosion(void);
	void saturation(void);
	void hue(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Augment*) This)->update();
		return NULL;
	}

public:
	int m_bgNoiseMean;
	int m_bgNoiseDev;
	int m_bgNoiseType;

	int m_nRot;

	double m_dCrop;
	int m_nCrop;

	double m_dShrink;
	int m_nShrink;

	double m_minLowResolution;
	double m_dLowResolution;
	int m_nLowResolution;

	int m_dNoise;
	int m_nNoise;
	int m_noiseType;

	double m_dContrast;
	int m_nContrast;

	double m_dBrightness;
	int m_nBrightness;

	double m_dBlur;
	int m_nBlur;

	double m_dGaussianBlur;
	int m_nGaussianBlur;

	double m_dMedianBlur;
	int m_nMedianBlur;

	double m_dBilateralBlur;
	int m_nBilateralBlur;

	vector<string> m_vCmd;
	bool m_bDeleteOriginal;
	bool m_bSaveOriginalCopy;
	double m_progress;

	Frame* m_pFrameIn;
	Frame* m_pFrameOut;
};
}

#endif
