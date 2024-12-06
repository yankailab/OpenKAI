#include "_APmavlink_avoid.h"

namespace kai
{

	_APmavlink_avoid::_APmavlink_avoid()
	{
		m_pAP = nullptr;
		m_pDet = nullptr;
		m_pMavlink = nullptr;
	}

	_APmavlink_avoid::~_APmavlink_avoid()
	{
	}

	int _APmavlink_avoid::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("APcopter_base", &n);
		m_pAP = (_APmavlink_base *)pK->findModule(n);

		n = "";
		pK->v("_Mavlink", &n);
		m_pMavlink = (_Mavlink *)pK->findModule(n);
		NULL__(m_pMavlink, OK_ERR_NOT_FOUND);

		n = "";
		pK->v("_DetectorBase", &n);
		m_pDet = (_DetectorBase *)pK->findModule(n);
		NULL__(m_pDet, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _APmavlink_avoid::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_avoid::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);
		NULL__(m_pDet, OK_ERR_NULLPTR);
		NULL__(m_pMavlink, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _APmavlink_avoid::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateTarget();
		}
	}

	void _APmavlink_avoid::updateTarget(void)
	{
		IF_(check() != OK_OK);

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

		m_pAP->setMode(AP_COPTER_LOITER);
		LOG_I("Set Loiter");
	}

	void _APmavlink_avoid::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(check() != OK_OK);

		string msg = "nTarget=" + i2str(m_pDet->getU()->size());
		((_Console *)pConsole)->addMsg(msg);
	}

	void _APmavlink_avoid::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() != OK_OK);

		Frame *pF = (Frame *)pFrame;
		Mat *pM = pF->m();
		IF_(pM->empty());

		IF_(m_obs.getTopClass() < 0);

		Rect r = bb2Rect(m_obs.getBB2Dscaled(pM->cols, pM->rows));
		rectangle(*pM, r, Scalar(0, 0, 255), 3);
	}

}
