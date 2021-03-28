#include "_AP_land.h"

#ifdef USE_OPENCV

namespace kai
{

_AP_land::_AP_land()
{
    m_zrK = 1.0;
	m_targetAlt = 1.0;
	m_dTarget = -1.0;
}

_AP_land::~_AP_land()
{
}

bool _AP_land::init(void* pKiss)
{
	IF_F(!this->_AP_follow::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("targetAlt", &m_targetAlt);
	pK->v("zrK", &m_zrK);

	return true;
}

bool _AP_land::start(void)
{
    NULL_F( m_pT );
	return m_pT->start(getUpdate, this);
}

int _AP_land::check(void)
{
	return this->_AP_follow::check();
}

void _AP_land::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();
		this->_StateBase::update();

		if (!updateTarget())
		{
			m_dTarget = -1.0;
		}

		m_pT->autoFPSto();
	}
}

bool _AP_land::updateTarget(void)
{
	IF_F(check() < 0);
    IF_F(!bActive());

	m_bTarget = findTarget();
    
	if (m_apMount.m_bEnable)
		m_pAP->setMount(m_apMount);

	if(!m_bTarget)
        m_vP = m_vTargetP;

	setPosLocal();
	return true;
}

bool _AP_land::findTarget(void)
{
	IF_F(check() < 0);

	//target
	_Object* tO = NULL;
	_Object* pO;
    float minW = FLT_MAX;
	int i = 0;
	while ((pO = m_pU->get(i++)) != NULL)
	{
		if(m_iClass >= 0)
		{
			IF_CONT(pO->getTopClass() != m_iClass);
		}

        float w = pO->getWidth();
		IF_CONT(w > minW);

		tO = pO;
        minW = w;
	}

	float *pX, *pY, *pR, *pH;
	float dTs = m_pT->getDt() * USEC_2_SEC;

	if(tO)
	{
		float x = tO->getX();
		float y = tO->getY();
		float r = tO->getRadius();
		float h = tO->getRoll();

		pX = m_fX.update(&x, dTs);
		pY = m_fY.update(&y, dTs);
		pR = m_fR.update(&r, dTs);
		pH = m_fH.update(&h, dTs);
	}
	else
	{
		pX = m_fX.update(NULL, dTs);
		pY = m_fY.update(NULL, dTs);
		pR = m_fR.update(NULL, dTs);
		pH = m_fH.update(NULL, dTs);
	}

	NULL_F(pX);

	//position
	m_vP.x = *pX;
	m_vP.y = *pY;
    float dX = m_vP.x - m_vTargetP.x;
	float dY = m_vP.y - m_vTargetP.y;
	float r = sqrt(dX*dX + dY*dY);
    m_vP.z = m_vTargetP.z * constrain(1.0 - r*m_zrK, 0.0, 1.0);

	//heading
    m_vP.w = *pH;

	//distance
//	m_filter.input(tO->getZ());
//	m_dTarget = m_filter.v();

	return true;
}

void _AP_land::draw(void)
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
	addMsg("Size=(" + f2str(m_vTargetBB.width()) + ", " + f2str(m_vTargetBB.height()) + ")", 1);

	IF_(!checkWindow());
	Mat* pMat = ((_WindowCV*) this->m_pWindow)->getFrame()->m();

	Rect r = bb2Rect(bbScale(m_vTargetBB, pMat->cols, pMat->rows));
	rectangle(*pMat, r, Scalar(0,0,255), 2);
}

}
#endif
