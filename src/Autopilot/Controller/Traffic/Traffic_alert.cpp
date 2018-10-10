#include "Traffic_alert.h"

namespace kai
{

Traffic_alert::Traffic_alert()
{
	m_pTB = NULL;
	m_tStampOB = 0;
	m_obj.reset();

	m_nAlert = 1;
	m_nMed = 3;
}

Traffic_alert::~Traffic_alert()
{
}

bool Traffic_alert::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK, nAlert);
	KISSm(pK,nMed);
	m_fNalert.init(m_nMed,3);

	m_alert.init();
	int pClassList[OBJECT_N_CLASS];
	int m_nClass = pK->array("classList", pClassList, OBJECT_N_CLASS);
	IF_F(m_nClass <= 0);
	for(int i=0; i<m_nClass; i++)
	{
		m_alert.addClass(pClassList[i]);
	}

	//link
	string iName;
	iName = "";
	pK->v("Traffic_base", &iName);
	m_pTB = (Traffic_base*) (pK->parent()->getChildInst(iName));

	return true;
}

int Traffic_alert::check(void)
{
	NULL__(m_pTB,-1);
	NULL__(m_pTB->m_pOB,-1);

	return this->ActionBase::check();
}

void Traffic_alert::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	_ObjectBase* pOB = m_pTB->m_pOB;
	IF_(pOB->m_tStamp <= m_tStampOB);
	m_tStampOB = pOB->m_tStamp;

	OBJECT* pO;
	int i=0;
	while((pO = pOB->at(i++)) != NULL)
	{
		IF_CONT(!m_pTB->bInsideROI(pO->m_bb));
		IF_CONT(!m_alert.bAlert(pO->m_topClass));

		//find if inside a car
		OBJECT* pT;
		OBJECT* pOverlap = NULL;
		int j=0;
		while((pT = pOB->at(j++)) != NULL)
		{
			IF_CONT(j==i);
			IF_CONT(pT->m_bb.x > pO->m_bb.x ||
					pT->m_bb.y > pO->m_bb.y ||
					pT->m_bb.z < pO->m_bb.z ||
					pT->m_bb.w < pO->m_bb.w);

			pOverlap = pT;
			break;
		}

		IF_CONT(pOverlap);
		m_obj.add(pO);
	}

	m_obj.update();
	m_fNalert.input(m_obj.size());
}

bool Traffic_alert::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = "nAlert=" + i2str(m_obj.size());
	pWin->addMsg(&msg);

	Scalar col = Scalar(0, 0, 255);
	vInt2 cSize;
	cSize.x = pMat->cols;
	cSize.y = pMat->rows;

	vInt4 iBB;
	Point pC;
	Rect r;
	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		iBB = pO->iBBox(cSize);
		pC = Point(iBB.midX(), iBB.midY());
		vInt42rect(iBB, r);
		rectangle(*pMat, r, col, 3);
	}

	if(m_fNalert.v() > (double)m_nAlert)
	{
		msg = "INTRUSION/ACCIDENT";
		putText(*pMat, msg,
				Point(0.3 * cSize.x, 0.8 * cSize.y),
				FONT_HERSHEY_SIMPLEX, 5.0, col, 10);
	}

	return true;
}

bool Traffic_alert::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	NULL_F(m_pTB);
	_ObjectBase* pOB = m_pTB->m_pOB;
	NULL_F(pOB);

	int nAlert = m_obj.size();
	string msg = "nAlert=" + i2str(nAlert);

	if(nAlert <= 0)
	{
		COL_MSG;
		iY++;
		mvaddstr(iY, CLI_X_MSG, msg.c_str());
		return true;
	}

	COL_ERROR;
	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		msg = pOB->getClassName(pO->m_topClass);
		iY++;
		mvaddstr(iY, CLI_X_MSG, msg.c_str());
	}

	return true;
}

}
