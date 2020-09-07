#include "_UItransform.h"

namespace kai
{

_UItransform::_UItransform()
{
	m_pIO = NULL;
}

_UItransform::~_UItransform()
{
}

bool _UItransform::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->getInst(iName));
	NULL_Fl(m_pIO,"_IOBase not found");

	vector<string> vP;
	pK->a("vPCB", &vP);
	IF_F(vP.empty());
	for(string p : vP)
	{
		_PointCloudBase* pPCB = (_PointCloudBase*) (pK->getInst(p));
		IF_CONT(!pPCB);

		m_vPCB.push_back(pPCB);
	}
	IF_F(m_vPCB.empty());

	return true;
}

bool _UItransform::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _UItransform::check(void)
{
	NULL__(m_pIO, -1);
	IF__(!m_pIO->isOpen(), -1);

	return 0;
}

void _UItransform::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		recv();

		this->autoFPSto();
	}
}

bool _UItransform::recv()
{
	IF_F(check()<0);

	static string s_strB = "";
	unsigned char B;
	int nB;

	while ((nB = m_pIO->read(&B, 1)) > 0)
	{
		s_strB += B;
		IF_CONT(B != ';');

		s_strB.erase(s_strB.length()-1, 1);
		handleMsg(s_strB);

		LOG_I("Received: " + s_strB);
		s_strB.clear();
	}

	return false;
}

void _UItransform::handleMsg(string& str)
{
	vector<string> vP = splitBy(str, ',');
	IF_(vP.size() < 7);

	int iModel = stoi(vP[0]);
	IF_(iModel >= m_vPCB.size());

	_PointCloudBase* pPCB = m_vPCB[iModel];

	vFloat3 v;
	v.init(stof(vP[1]), stof(vP[2]), stof(vP[3]));
	pPCB->setTranslation(v);
	v.init(stof(vP[4]), stof(vP[5]), stof(vP[6]));
	pPCB->setRotation(v);

}

void _UItransform::draw(void)
{
	this->_ThreadBase::draw();

	string msg = *this->getName();
	if (m_pIO->isOpen())
		msg += ": CONNECTED";
	else
		msg += ": Not connected";

	addMsg(msg);

}

}
