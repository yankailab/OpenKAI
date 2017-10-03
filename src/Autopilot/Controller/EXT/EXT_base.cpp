#include "EXT_base.h"

namespace kai
{

EXT_base::EXT_base()
{
	m_pIO = NULL;
	m_pDB = NULL;
	m_iTargetClass = 0;
}

EXT_base::~EXT_base()
{
}

bool EXT_base::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK,iTargetClass);

	return true;
}

bool EXT_base::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pDB = (_DetectorBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void EXT_base::update(void)
{
	this->ActionBase::update();
	NULL_(m_pDB);
	NULL_(m_pIO);
	IF_(!m_pIO->isOpen());
	IF_(!isActive());

	OBJECT* pO;
	int i;
	for(i=0; i<m_pDB->size(); i++)
	{
		pO = m_pDB->at(i);
		IF_(!pO);
		IF_CONT(pO->m_iClass != m_iTargetClass);

		string one = "";
		one += i2str(pO->m_bbox.x);
		one += ",";
		one += i2str(pO->m_bbox.y);
		one += ",";
		one += i2str(pO->m_bbox.z - pO->m_bbox.x);
		one += ",";
		one += i2str(pO->m_bbox.w - pO->m_bbox.y);
		one += ",";
		one += i2str((int)(pO->m_prob*100));
		one += ";";
		m_pIO->write((unsigned char*)one.c_str(),one.length());

		LOG_I(one);
	}
}

bool EXT_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	NULL_F(pMat);
	IF_F(pMat->empty());

	return true;
}



}
