/*
 * _PCtransform.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCtransform.h"

#include <Eigen/Geometry>

namespace
{
	Eigen::Matrix3d rotationMatrixFromXYZ(const Eigen::Vector3d &rotation)
	{
		return Eigen::AngleAxisd(rotation(0), Eigen::Vector3d::UnitX()).toRotationMatrix() *
			   Eigen::AngleAxisd(rotation(1), Eigen::Vector3d::UnitY()).toRotationMatrix() *
			   Eigen::AngleAxisd(rotation(2), Eigen::Vector3d::UnitZ()).toRotationMatrix();
	}

	Eigen::Matrix3d rotationMatrixFromAxisAngle(const Eigen::Vector3d &rotation)
	{
		const double phi = rotation.norm();
		if (phi > 0.0)
			return Eigen::AngleAxisd(phi, rotation / phi).toRotationMatrix();

		return Eigen::Matrix3d::Identity();
	}

	Eigen::Matrix3d rotationMatrixFromQuaternion(const Eigen::Vector4d &rotation)
	{
		return Eigen::Quaterniond(rotation(0), rotation(1), rotation(2), rotation(3))
			.normalized()
			.toRotationMatrix();
	}
}

namespace kai
{

	_PCtransform::_PCtransform()
	{
		m_vT.set(0);
		m_vR.set(0);
		m_vQ.clear();
		m_mT = Matrix4d::Identity();
		m_A = Matrix4d::Identity();
	}

	_PCtransform::~_PCtransform()
	{
	}

	bool _PCtransform::init(const json &j)
	{
		IF_F(!this->_PointCloud::init(j));

		jKv(j, "dTexpire", m_dTexpire);

		jKv<double>(j, "vT", m_vT);
		jKv<double>(j, "vR", m_vR);
		setTranslation(m_vT);
		setRotation(m_vR);
		updateTranslationMatrix(false);

		loadConfig();

		return true;
	}

	bool _PCtransform::loadConfig(json *pJ, string fName)
	{
		json j;
		IF_F(!this->BASE::loadConfig(&j, fName));

		const json &jG = jK(j, "_PCtransform");
		if (jG.is_object())
		{
			vDouble3 vT, vR;
			vT.clear();
			vR.clear();
			jKv<double>(jG, "vT", vT);
			jKv<double>(jG, "vR", vR);

			setTranslation(vT);
			setRotation(vR);
			updateTranslationMatrix(false);

			vector<double> vmT;
			jKv(jG, "mT", vmT);
			if (vmT.size() >= 16)
			{
				Matrix4d mT;
				mT(0, 0) = vmT[0];
				mT(0, 1) = vmT[1];
				mT(0, 2) = vmT[2];
				mT(0, 3) = vmT[3];

				mT(1, 0) = vmT[4];
				mT(1, 1) = vmT[5];
				mT(1, 2) = vmT[6];
				mT(1, 3) = vmT[7];

				mT(2, 0) = vmT[8];
				mT(2, 1) = vmT[9];
				mT(2, 2) = vmT[10];
				mT(2, 3) = vmT[11];

				mT(3, 0) = vmT[12];
				mT(3, 1) = vmT[13];
				mT(3, 2) = vmT[14];
				mT(3, 3) = vmT[15];

				setTranslationMatrix(mT);
			}
		}

		if (pJ)
		{
			*pJ = j;
		}
		return true;
	}

	bool _PCtransform::saveConfig(json &j, string fName)
	{
		json jG = json::object();
		jG["vT"] = {m_vT.x, m_vT.y, m_vT.z};
		jG["vR"] = {m_vR.x, m_vR.y, m_vR.z};

		Matrix4d mT = getTranslationMatrix();
		jG["mT"] = {mT(0, 0), mT(0, 1), mT(0, 2), mT(0, 3),
					mT(1, 0), mT(1, 1), mT(1, 2), mT(1, 3),
					mT(2, 0), mT(2, 1), mT(2, 2), mT(2, 3),
					mT(3, 0), mT(3, 1), mT(3, 2), mT(3, 3)};

		j["_PCtransform"] = jG;

		return this->BASE::saveConfig(j, fName);
	}

    bool _PCtransform::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_ModuleBase::link(j, pM));

        string n = "";
        jKv(j, "_PointCloud", n);
        m_pPS = (_PointCloud *)(pM->findModule(n));

        return true;
    }

	bool _PCtransform::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _PCtransform::check(void)
	{
		NULL_F(m_pPS);
		return this->_PointCloud::check();
	}

	void _PCtransform::clear(void)
	{
		this->_PointCloud::clear();
	}

	void _PCtransform::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateTransform();
		}
	}

	void _PCtransform::updateTransform(void)
	{
		IF_(!check());

		uint64_t tExpire = 0;
		if(m_dTexpire > 0)
			tExpire = getApproxTbootUs() - m_dTexpire;

		m_pPS->get(&m_grPt, tExpire);
	}

	void _PCtransform::setTranslation(const vDouble3 &vT)
	{
		m_vT = vT;
	}

	void _PCtransform::setRotation(const vDouble3 &vR)
	{
		m_vR = vR;
	}

	void _PCtransform::setQuaternion(const vDouble4 &vQ)
	{
		m_vQ = vQ;
	}

	void _PCtransform::updateTranslationMatrix(bool bUseQuaternion, vDouble3 *pRa)
	{
		if (bUseQuaternion)
			m_mT = createTranslationMatrix(m_vT, m_vQ, pRa);
		else
			m_mT = createTranslationMatrix(m_vT, m_vR, pRa);

		m_A = m_mT;
	}

	Matrix4d _PCtransform::createTranslationMatrix(const vDouble3 &vT, const vDouble3 &vR, vDouble3 *pRa)
	{
		Matrix4d mT = Matrix4d::Identity();
		Vector3d eR(vR.x, vR.y, vR.z);
		mT.block(0, 0, 3, 3) = rotationMatrixFromXYZ(eR);
		mT(0, 3) = vT.x;
		mT(1, 3) = vT.y;
		mT(2, 3) = vT.z;

		NULL__(pRa, mT);

		eR = Vector3d(pRa->x, pRa->y, pRa->z);
		Matrix3d mR = rotationMatrixFromAxisAngle(eR);
		Matrix3d mTr = mT.block(0, 0, 3, 3);
		mT.block(0, 0, 3, 3) = mTr * mR;

		return mT;
	}

	Matrix4d _PCtransform::createTranslationMatrix(const vDouble3 &vT, const vDouble4 &vQ, vDouble3 *pRa)
	{
		Matrix4d mT = Matrix4d::Identity();
		Vector4d eQ(vQ.x, vQ.y, vQ.z, vQ.w);
		mT.block(0, 0, 3, 3) = rotationMatrixFromQuaternion(eQ);
		mT(0, 3) = vT.x;
		mT(1, 3) = vT.y;
		mT(2, 3) = vT.z;

		NULL__(pRa, mT);

		Vector3d eR(pRa->x, pRa->y, pRa->z);
		Matrix3d mR = rotationMatrixFromAxisAngle(eR);
		Matrix3d mTr = mT.block(0, 0, 3, 3);
		mT.block(0, 0, 3, 3) = mR * mTr;

		return mT;
	}

	Matrix4d _PCtransform::getTranslationMatrix(void)
	{
		return m_mT;
	}

	void _PCtransform::setTranslationMatrix(const Matrix4d &mT)
	{
		m_mT = mT;
		m_A = m_mT;
	}

	vDouble3 _PCtransform::getTranslation(void)
	{
		return m_vT;
	}

	vDouble3 _PCtransform::getRotation(void)
	{
		return m_vR;
	}

	vDouble4 _PCtransform::getQuaternion(void)
	{
		return m_vQ;
	}

    void _PCtransform::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        pC->addMsg("vT = (" + f2str(m_vT.x) + "," + f2str(m_vT.y) + ", " + f2str(m_vT.z) + ")");
        pC->addMsg("vR = (" + f2str(m_vR.x) + "," + f2str(m_vR.y) + ", " + f2str(m_vR.z) + ")");
        pC->addMsg("vQ = (" + f2str(m_vQ.x) + "," + f2str(m_vQ.y) + ", " + f2str(m_vQ.z) + ", " + f2str(m_vQ.w) + ")");
    }


}
