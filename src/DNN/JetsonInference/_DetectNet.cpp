/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_DetectNet.h"

namespace kai
{

	_DetectNet::_DetectNet()
	{
		m_pRGBA = nullptr;
		m_pRGBAf = nullptr;

		m_pDN = nullptr;
		m_nBox = 0;
		m_nClass = 0;
		m_type = detectNet_uff;
		m_thr = 0.5;
		m_layerIn = "Input";
		m_layerOut = "NMS";
		m_layerNboxOut = "NMS_1";
		m_vDimIn.set(3, 300, 300);
		m_nMaxBatch = DEFAULT_MAX_BATCH_SIZE;
		m_precision = TYPE_FASTEST;
		m_device = DEVICE_GPU;
		m_bAllowGPUfallback = true;

		m_bSwapRB = false;
		m_vMean.init();
	}

	_DetectNet::~_DetectNet()
	{
		DEL(m_pRGBA);
		DEL(m_pRGBAf);
	}

	int _DetectNet::init(void *pKiss)
	{
		CHECK_(this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("thr", &m_thr);
		pK->v("bSwapRB", &m_bSwapRB);
		pK->v("vMean", &m_vMean);
		pK->v("type", (int *)&m_type);

		m_pRGBA = new Frame();
		m_pRGBAf = new Frame();

		return OK_OK;
	}

	int _DetectNet::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _DetectNet::check(void)
	{
		NULL__(m_pV, OK_ERR_NULLPTR);
		NULL__(m_pU, OK_ERR_NULLPTR);
		NULL__(m_pDN, OK_ERR_NULLPTR);
		Frame *pBGR = m_pV->getFrameRGB();
		NULL__(pBGR, OK_ERR_NULLPTR);
		IF__(pBGR->bEmpty(), OK_ERR_NOT_READY);

		return this->_DetectorBase::check();
	}

	bool _DetectNet::open(void)
	{
		if (m_type == detectNet_uff)
		{
			m_pDN = detectNet::Create(m_fModel.c_str(),
									  m_fClass.c_str(),
									  m_thr,
									  m_layerIn.c_str(),
									  Dims3(m_vDimIn.x, m_vDimIn.y, m_vDimIn.z),
									  m_layerOut.c_str(),
									  m_layerNboxOut.c_str(),
									  m_nMaxBatch,
									  (precisionType)m_precision,
									  (deviceType)m_device,
									  m_bAllowGPUfallback);
		}

		NULL_F(m_pDN);

		m_nClass = m_pDN->GetNumClasses();
		m_pDN->SetThreshold(m_thr);

		return true;
	}

	void _DetectNet::update(void)
	{
		open();

		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			detect();
			m_pU->swap();

		}
	}

	void _DetectNet::detect(void)
	{
		IF_(check() != OK_OK);

		*m_pRGBA = m_pV->getFrameRGB()->cvtColor(CV_BGR2RGBA);
		*m_pRGBAf = m_pRGBA->cvtTo(CV_32FC4);
		GpuMat fGMat = *m_pRGBAf->gm();

		detectNet::Detection *pBox = NULL;
		m_nBox = m_pDN->Detect((float *)fGMat.data, fGMat.cols, fGMat.rows, IMAGE_RGBA32F, &pBox, detectNet::OVERLAY_NONE);
		IF_(m_nBox <= 0);

		float kx = 1.0 / (float)fGMat.cols;
		float ky = 1.0 / (float)fGMat.rows;

		for (int i = 0; i < m_nBox; i++)
		{
			detectNet::Detection *pB = &pBox[i];

			_Object o;
			o.clear();
			o.setTstamp(m_pT->getTfrom());
			o.setTopClass(pB->ClassID, pB->Confidence);
			string txt(m_pDN->GetClassDesc(pB->ClassID));
			o.setText(txt);
			o.setRect(pB->Left, pB->Top, pB->Width(), pB->Height());
			o.scalePosDim(kx, ky);

			m_pU->add(o);
			LOG_I("BBox: " << o.getText() << " Prob: " << f2str(o.getTopClassProb()));
		}
	}

}
