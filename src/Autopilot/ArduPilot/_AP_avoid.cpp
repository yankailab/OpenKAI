#include "../ArduPilot/_AP_avoid.h"

namespace kai
{

	_AP_avoid::_AP_avoid()
	{
		m_pAP = NULL;
		m_pDet = NULL;
		m_pMavlink = NULL;
	}

	_AP_avoid::~_AP_avoid()
	{
	}

	bool _AP_avoid::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
    	

		string n;
		n = "";
		F_INFO(pK->v("APcopter_base", &n));
		m_pAP = (_AP_base *)pK->getInst(n);

		n = "";
		F_ERROR_F(pK->v("_Mavlink", &n));
		m_pMavlink = (_Mavlink *)pK->getInst(n);
		NULL_Fl(m_pMavlink, n + ": not found");

		n = "";
		F_ERROR_F(pK->v("_DetectorBase", &n));
		m_pDet = (_DetectorBase *)pK->getInst(n);
		NULL_Fl(m_pDet, n + ": not found");

		return true;
	}

	bool _AP_avoid::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_avoid::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pDet, -1);
		NULL__(m_pMavlink, -1);

		return this->_ModuleBase::check();
	}

	void _AP_avoid::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			updateTarget();

			m_pT->autoFPSto();
		}
	}

	void _AP_avoid::updateTarget(void)
	{
		IF_(check() < 0);

		_Object o;
		o.clear();
		_Object *pO;
		int i = 0;
		while ((pO = m_pDet->getU()->get(i++)) != NULL)
		{
			o = *pO;
			o.setTopClass(0);
		}

		if (o.getTopClass() < 0)
		{
			m_obs.clear();
			LOG_I("Target not found");
			return;
		}

		m_obs = o;

		if (m_pAP->m_apMode == AP_COPTER_LOITER)
		{
			LOG_I("Already Loiter");
			return;
		}

		m_pAP->setApMode(AP_COPTER_LOITER);
		LOG_I("Set Loiter");
	}

	void _AP_avoid::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(check() < 0);

		string msg = "nTarget=" + i2str(m_pDet->getU()->size());
		((_Console *)pConsole)->addMsg(msg);
	}

	void _AP_avoid::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		Frame *pF = (Frame *)pFrame;
		Mat *pM = pF->m();
		IF_(pM->empty());

		IF_(m_obs.getTopClass() < 0);

		Rect r = bb2Rect(m_obs.getBB2Dscaled(pM->cols, pM->rows));
		rectangle(*pM, r, Scalar(0, 0, 255), 3);
	}

}
