#include "_APcopter_avoid.h"

namespace kai
{

_APcopter_avoid::_APcopter_avoid()
{
	m_pAP = NULL;
	m_pDet = NULL;
	m_pMavlink = NULL;
}

_APcopter_avoid::~_APcopter_avoid()
{
}

bool _APcopter_avoid::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (_APcopter_base*) (pK->parent()->getChildInst(iName));

	iName = "";
	F_ERROR_F(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pMavlink, iName+": not found");

	return true;
}

int _APcopter_avoid::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pDet, -1);
	NULL__(m_pMavlink, -1);

	return 0;
}

void _APcopter_avoid::update(void)
{
	this->_AutopilotBase::update();
	IF_(check()<0);

	OBJECT o;
	o.init();
	OBJECT* pO;
	int i=0;
	while((pO = m_pDet->at(i++)) != NULL)
	{
		o = *pO;
		o.m_topClass = 0;
	}

	IF_(o.m_topClass<0);
	m_obs = o;

	IF_(m_pAP->m_apMode == LOITER);
	m_pAP->setApMode(LOITER);
}

void _APcopter_avoid::draw(void)
{
	this->_AutopilotBase::draw();

	string msg = "nTarget=" + i2str(m_pDet->size());
	addMsg(msg);

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Rect r = convertBB<vInt4>(convertBB(m_obs.m_bb, cs));
	rectangle(*pMat, r, Scalar(0,0,255), 3);

}

}
