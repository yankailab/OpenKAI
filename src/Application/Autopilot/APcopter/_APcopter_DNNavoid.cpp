#include "_APcopter_DNNavoid.h"

namespace kai
{

_APcopter_DNNavoid::_APcopter_DNNavoid()
{
#ifdef USE_TENSORRT
	m_pIN = NULL;
#endif

	m_pAP = NULL;
	m_nVision = 0;
	m_action = DA_UNKNOWN;
}

_APcopter_DNNavoid::~_APcopter_DNNavoid()
{
}

bool _APcopter_DNNavoid::init(void* pKiss)
{
	IF_F(!this->_ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (_APcopter_base*) (pK->parent()->getChildInst(iName));

#ifdef USE_TENSORRT
	iName = "";
	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pIN, iName << " not found");

	while(!m_pIN->bReady())
	{
		sleep(1);
	}

	Kiss** pItr = pK->getChildItr();
	OBJECT tO;
	tO.init();
	tO.m_fBBox.z = 1.0;
	tO.m_fBBox.w = 1.0;

	m_nVision = 0;
	while (pItr[m_nVision])
	{
		IF_F(m_nVision >= DNNAVOID_N_VISION);

		Kiss* pKT = pItr[m_nVision];
		DNN_AVOID_VISION* pV = &m_pVision[m_nVision];
		m_nVision++;
		pV->init();

		F_ERROR_F(pKT->v("orientation", (int* )&pV->m_orientation));
		F_INFO(pKT->v("rMin", &pV->m_rMin));
		F_INFO(pKT->v("rMax", &pV->m_rMax));
		F_INFO(pKT->v("l", &tO.m_fBBox.x));
		F_INFO(pKT->v("t", &tO.m_fBBox.y));
		F_INFO(pKT->v("r", &tO.m_fBBox.z));
		F_INFO(pKT->v("b", &tO.m_fBBox.w));

		double deg;
		if(pKT->v("angleDeg", &deg))
		{
			pV->m_angleTan = tan(deg*DEG_RAD);
		}

		pV->m_pObj = m_pIN->add(&tO);
		NULL_F(pV->m_pObj);
		m_pIN->m_obj.update(); //TOOD:

		string pClass[DETECTOR_N_CLASS];
		Kiss** pItrAct = pKT->getChildItr();
		pV->m_nAction = 0;
		int i=0;
		while (pItrAct[i])
		{
			IF_Fl(pV->m_nAction >= DNNAVOID_N_ACTION, "Exceeded the max number of action:" << DNNAVOID_N_ACTION);

			Kiss* pKA = pItrAct[i++];
			int nClass = pKA->array("class", pClass, DETECTOR_N_CLASS);
			IF_CONT(nClass <= 0);

			DNN_AVOID_ACTION* pA = &pV->m_pAction[pV->m_nAction++];
			pA->init();

			for(int j=0; j<nClass; j++)
			{
				pA->addClass(m_pIN->getClassIdx(pClass[j]));
			}
			IF_Fl(pA->m_mClass == 0, "nClass = 0");

			string strAction = "";
			IF_Fl(!pKA->v("action", &strAction), "Action not defined");

			pA->m_action = str2actionType(strAction);
			IF_Fl(pA->m_action == DA_UNKNOWN, "Unknown action: " << strAction);

		}
	}
#endif

	return true;
}

DNN_AVOID_ACTION_TYPE _APcopter_DNNavoid::str2actionType(string& strAction)
{
	if (strAction == "safe")
		return DA_SAFE;
	else if (strAction == "warn")
		return DA_WARN;
	else if (strAction == "forbid")
		return DA_FORBID;

	return DA_UNKNOWN;
}

string _APcopter_DNNavoid::actionType2str(DNN_AVOID_ACTION_TYPE aType)
{
	static const string pType[] =
	{ "unknown", "safe", "warn", "forbid" };
	return pType[aType];
}

void _APcopter_DNNavoid::update(void)
{
	this->_ActionBase::update();

#ifdef USE_TENSORRT
	NULL_(m_pIN);
	IF_(!m_pIN->bReady());
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	_Mavlink* pMavlink = m_pAP->m_pMavlink;
	double alt = (double)pMavlink->m_msg.global_position_int.relative_alt;

	int i,j;
	for (i = 0; i < m_nVision; i++)
	{
		DNN_AVOID_VISION* pV = &m_pVision[i];

		for (j = 0; j < pV->m_nAction; j++)
		{
			DNN_AVOID_ACTION* pA = &pV->m_pAction[j];
			IF_CONT(!(pA->m_mClass & pV->m_pObj->m_mClass));

			m_action = pA->m_action;

			if(m_action <= DA_SAFE)
			{
				LOG_I("SAFE: " << m_pIN->getClassName(pV->m_pObj->m_iClass));
				break;
			}

			if(m_action <= DA_WARN)
			{
				LOG_I("WARNING: " << m_pIN->getClassName(pV->m_pObj->m_iClass));
				break;
			}

			LOG_I("FORBID: " << m_pIN->getClassName(pV->m_pObj->m_iClass));

			pMavlink->distanceSensor(0, //type
					pV->m_orientation,	//orientation
					pV->m_rMax,
					pV->m_rMin,
					constrain(alt*pV->m_angleTan, pV->m_rMin, pV->m_rMax));

			break;
		}

		if(m_action <= DA_WARN)
		{
			pMavlink->distanceSensor(0, //type
					pV->m_orientation,	//orientation
					pV->m_rMax,
					pV->m_rMin,
					pV->m_rMax);
		}
	}
#endif

}

bool _APcopter_DNNavoid::draw(void)
{
	IF_F(!this->_ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

}
