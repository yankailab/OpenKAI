#include "APMcopter_guided.h"

namespace kai
{

APMcopter_guided::APMcopter_guided()
{
	ActionBase();

	m_pSF40 = NULL;
	m_pAPM = NULL;
}

APMcopter_guided::~APMcopter_guided()
{
}

bool APMcopter_guided::init(void* pKiss)
{
	CHECK_F(this->ActionBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

//	F_INFO(pK->v("", &m_guidedTarget.m_orientX));


	return true;
}

bool APMcopter_guided::link(void)
{
	CHECK_F(this->ActionBase::link()==false);
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";

	F_INFO(pK->v("APMcopter_base", &iName));
	m_pAPM = (APMcopter_base*) (pK->root()->getChildInstByName(&iName));

	F_INFO(pK->v("_Lightware_SF40", &iName));
	m_pSF40 = (_Lightware_SF40*) (pK->root()->getChildInstByName(&iName));

	//Add GPS

	return true;
}

void APMcopter_guided::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAPM);
	CHECK_(m_pAM->getCurrentStateIdx() != m_iActiveState);

	updateAttitude();

}

void APMcopter_guided::updateAttitude(void)
{
	NULL_(m_pAPM->m_pMavlink);
	NULL_(m_pSF40);

	//TODO: set new holding position in mode change (Any -> Guided)

	APMcopter_PID* pPID;
	APMcopter_CTRL* pCtrl;
	double v;

	//roll
	pPID = &m_pAPM->m_pidRoll;
	pCtrl = &m_pAPM->m_ctrlRoll;
	v = m_pSF40->m_pX->v();

	pCtrl->m_errOld = pCtrl->m_err;
	pCtrl->m_predPos = v + (v-pCtrl->m_pos)*pPID->m_dT;
	pCtrl->m_pos = v;
	pCtrl->m_err = pCtrl->m_targetPos - pCtrl->m_predPos;
	pCtrl->m_errInteg += pCtrl->m_err;
	pCtrl->m_v = pPID->m_P * pCtrl->m_err
						+ pPID->m_D * (pCtrl->m_err - pCtrl->m_errOld)
						+ constrain(pPID->m_I * pCtrl->m_errInteg, pPID->m_Imax, -pPID->m_Imax);

	//pitch
	pPID = &m_pAPM->m_pidPitch;
	pCtrl = &m_pAPM->m_ctrlPitch;
	v = m_pSF40->m_pY->v();

	pCtrl->m_errOld = pCtrl->m_err;
	pCtrl->m_predPos = v + (v-pCtrl->m_pos)*pPID->m_dT;
	pCtrl->m_pos = v;
	pCtrl->m_err = pCtrl->m_targetPos - pCtrl->m_predPos;
	pCtrl->m_errInteg += pCtrl->m_err;
	pCtrl->m_v = pPID->m_P * pCtrl->m_err
						+ pPID->m_D * (pCtrl->m_err - pCtrl->m_errOld)
						+ constrain(pPID->m_I * pCtrl->m_errInteg, pPID->m_Imax, -pPID->m_Imax);

	//TODO:
	//yaw
	//throttle

	//Send Mavlink command
	m_pAPM->updateAttitude();

}

bool APMcopter_guided::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);
	Mat* pMat = pFrame->getCMat();

	if(m_pAPM)
	{
		putText(*pFrame->getCMat(), "APMcopter_GUIDED: Roll="
									+ f2str(m_pAPM->m_ctrlRoll.m_v)
									+", Pitch="
									+ f2str(m_pAPM->m_ctrlPitch.m_v),
				cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
				Scalar(0, 255, 0), 1);
		pTextPos->m_y += pTextPos->m_w;
	}

	return true;
}


}
