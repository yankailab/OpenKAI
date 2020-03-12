#include "_AP_descent.h"

namespace kai
{

_AP_descent::_AP_descent()
{
	m_alt = 3.0;
	m_detSize = 0.25;
	m_vRDD.init(0.0, 0.0, 1.0, 1.0);
	m_dTarget = -1.0;
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

	int wLen = 3;
	pK->v("wLen", &wLen);
	m_filter.init(wLen, 2);

	return true;
}

bool _AP_descent::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AP_descent::check(void)
{
	return this->_AP_follow::check();
}

void _AP_descent::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AP_posCtrl::update();

		if (!updateTarget())
		{
			m_filter.reset();
			m_dTarget = -1.0;
		}

		this->autoFPSto();
	}
}

bool _AP_descent::updateTarget(void)
{
	IF_F(check() < 0);

	m_bTarget = findTarget();

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
		m_pMC->getMission()->complete();
		return false;
	}

	setPosLocal();
	return true;
}

bool _AP_descent::findTarget(void)
{
	IF_F(check() < 0);

	//target
	OBJECT* tO = NULL;
	OBJECT* pO;
	int iTag = -1;
	int i = 0;
	while ((pO = m_pDet->at(i++)) != NULL)
	{
		IF_CONT(pO->m_topClass <= iTag);

		tO = pO;
		iTag = pO->m_topClass;
	}

	NULL_F(tO);

	//position
	m_vTargetBB = tO->m_bb;
	m_vP.x = m_vTargetBB.midX();
	m_vP.y = m_vTargetBB.midY();

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
	m_vP.w = Hdg(m_pAP->getApHdg() + tO->m_angle);
	Land* pLand = (Land*)m_pMC->getMission();
	if(pLand->m_type == mission_land)
	{
		for(int i=0; i<pLand->m_vTag.size(); i++)
		{
			LAND_TAG* pTag = &pLand->m_vTag[i];
			IF_CONT(pTag->m_iTag != tO->m_topClass);

			m_vP.w = Hdg(m_vP.w + pTag->m_angle);
			break;
		}
	}

	//distance
	m_filter.input(tO->m_dist);
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

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Rect r = convertBB<vInt4>(convertBB(m_vTargetBB, cs));
	rectangle(*pMat, r, Scalar(0,0,255), 2);
}

}
