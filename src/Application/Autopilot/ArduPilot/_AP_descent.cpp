#include "_AP_descent.h"

#ifdef USE_OPENCV

namespace kai
{

_AP_descent::_AP_descent()
{
	m_alt = 3.0;
	m_detSize = 0.25;
	m_vRDD.init(0.0, 0.0, 1.0, 1.0);
	m_dTarget = -1.0;
	m_iRelayLED = 0;
}

_AP_descent::~_AP_descent()
{
}

bool _AP_descent::init(void* pKiss)
{
	IF_F(!this->_AP_follow::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("alt", &m_alt);
	pK->v("detSize", &m_detSize);
	pK->v("vRDD", &m_vRDD);
	pK->v("iRelayLED",&m_iRelayLED);

	int wLen = 3;
	pK->v("wLen", &wLen);
	m_filter.init(wLen, 2);

	return true;
}

bool _AP_descent::start(void)
{
    NULL_F( m_pT );
	return m_pT->start(getUpdate, this);
}

int _AP_descent::check(void)
{
	return this->_AP_follow::check();
}

void _AP_descent::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		this->_AP_posCtrl::update();

		if (!updateTarget())
		{
			m_filter.reset();
			m_dTarget = -1.0;
		}

		m_pT->autoFPSto();
	}
}

bool _AP_descent::updateTarget(void)
{
	IF_F(check() < 0);

	m_bTarget = findTarget();
	m_pAP->m_pMav->clDoSetRelay(m_iRelayLED, m_bTarget);

	IF_F(!bActive());

	if (m_apMount.m_bEnable)
		m_pAP->setMount(m_apMount);

	if(!m_bTarget)
	{
		releaseCtrl();
		return false;
	}

	float w = m_vTargetBB.width();
	float h = m_vTargetBB.height();
	if (m_dTarget > 0.0 && m_dTarget < m_alt && w > m_detSize && h > m_detSize)
	{
		m_pSC->getState()->complete();
		for(int i=0; i<10; i++)
		{
			releaseCtrl();
		}

		return false;
	}

	setPosLocal();
	return true;
}

bool _AP_descent::findTarget(void)
{
	IF_F(check() < 0);

	//target
	_Object* tO = NULL;
	_Object* pO;
	int iTag = -1;
	int i = 0;
	while ((pO = m_pDet->m_pU->get(i++)) != NULL)
	{
		IF_CONT(pO->getTopClass() <= iTag);

		tO = pO;
		iTag = pO->getTopClass();
	}

	NULL_F(tO);

	//position
	m_vP.x = tO->getX();
	m_vP.y = tO->getY();

	if (m_vP.x > m_vRDD.x && m_vP.x < m_vRDD.z
			&& m_vP.y > m_vRDD.y && m_vP.y < m_vRDD.w)
	{
		m_vP.z = m_vTargetP.z;
	}
	else
	{
		m_vP.z = 0.0;
	}

	//heading
	m_vP.w = Hdg(m_pAP->getApHdg() + tO->getRoll());
	Land* pLand = (Land*)m_pSC->getState();
	if(pLand->m_type == state_land)
	{
		for(LAND_TAG& tag : pLand->m_vTag)
		{
			IF_CONT(tag.m_iTag != tO->getTopClass());

			m_vP.w = Hdg(m_vP.w + tag.m_angle);
			break;
		}
	}

	//distance
	m_filter.input(tO->getZ());
	m_dTarget = m_filter.v();

	return true;
}

void _AP_descent::draw(void)
{
	this->_AP_follow::draw();
	IF_(check() < 0);

	if (!m_bTarget)
	{
		addMsg("Target not found", 1);
		return;
	}

	addMsg("Target");
	vFloat2 c = m_vTargetBB.center();
	addMsg("Pos=(" + f2str(c.x) + ", " + f2str(c.y) + "), d=" + f2str(m_dTarget), 1);
	addMsg("Size=(" + f2str(m_vTargetBB.width()) + ", " + f2str(m_vTargetBB.height()) + ")",	1);

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	Rect r = bb2Rect(bbScale(m_vTargetBB, pMat->cols, pMat->rows));
	rectangle(*pMat, r, Scalar(0,0,255), 2);
}

}
#endif
