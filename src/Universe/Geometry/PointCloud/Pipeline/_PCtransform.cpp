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
	Eigen::Matrix3d rotationMatrixFromXYZ(const Vector3d &rotation)
	{
		return Eigen::AngleAxisd(rotation(0), Vector3d::UnitX()).toRotationMatrix() *
			   Eigen::AngleAxisd(rotation(1), Vector3d::UnitY()).toRotationMatrix() *
			   Eigen::AngleAxisd(rotation(2), Vector3d::UnitZ()).toRotationMatrix();
	}

	Eigen::Matrix3d rotationMatrixFromAxisAngle(const Vector3d &rotation)
	{
		const double phi = rotation.norm();
		if (phi > 0.0)
			return Eigen::AngleAxisd(phi, rotation / phi).toRotationMatrix();

		return Eigen::Matrix3d::Identity();
	}

	Eigen::Matrix3d rotationMatrixFromQuaternion(const Vector4d &rotation)
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
		m_vT.setZero();
		m_vR.setZero();
		m_vQ.setZero();
		m_mT = Eigen::Matrix4d::Identity();
		m_A = Eigen::Matrix4d::Identity();
	}

	_PCtransform::~_PCtransform()
	{
	}

	bool _PCtransform::loadConfig(void)
	{
		IF_F(!this->_PointCloud::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "dTexpire", m_dTexpire);

		jKv<double>(j, "vT", m_vT);
		jKv<double>(j, "vR", m_vR);
		setTranslation(m_vT);
		setRotation(m_vR);
		updateTranslationMatrix(false);

		vector<double> vmT;
		jKv(j, "mT", vmT);
		if (vmT.size() >= 16)
		{
			Eigen::Matrix4d mT;
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

		return true;
	}

	bool _PCtransform::saveConfig(bool bExport)
	{
		IF_F(!_PointCloud::saveConfig(false));

		json &j = *m_pJ;
		j["dTexpire"] = m_dTexpire;
		j["vT"] = {m_vT.x(), m_vT.y(), m_vT.z()};
		j["vR"] = {m_vR.x(), m_vR.y(), m_vR.z()};

		const Eigen::Matrix4d mT = getTranslationMatrix();
		j["mT"] = {mT(0, 0), mT(0, 1), mT(0, 2), mT(0, 3),
					mT(1, 0), mT(1, 1), mT(1, 2), mT(1, 3),
					mT(2, 0), mT(2, 1), mT(2, 2), mT(2, 3),
					mT(3, 0), mT(3, 1), mT(3, 2), mT(3, 3)};

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

    bool _PCtransform::link(void)
    {
        IF_F(!this->_PointCloud::link());
        const json &j = *m_pJ;

        string n = "";
        jKv(j, "_PointCloud", n);
        m_pPS = (_PointCloud *)(m_pM->findModule(n));

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
		while (m_pT->bRun())
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
			tExpire = getTns() - m_dTexpire;

		m_pPS->get(&m_grPt, tExpire);
	}

	void _PCtransform::setTranslation(const Vector3d &vT)
	{
		m_vT = vT;
	}

	void _PCtransform::setRotation(const Vector3d &vR)
	{
		m_vR = vR;
	}

	void _PCtransform::setQuaternion(const Vector4d &vQ)
	{
		m_vQ = vQ;
	}

	void _PCtransform::updateTranslationMatrix(bool bUseQuaternion, Vector3d *pRa)
	{
		if (bUseQuaternion)
			m_mT = createTranslationMatrix(m_vT, m_vQ, pRa);
		else
			m_mT = createTranslationMatrix(m_vT, m_vR, pRa);

		m_A = m_mT;
	}

	Eigen::Matrix4d _PCtransform::createTranslationMatrix(const Vector3d &vT, const Vector3d &vR, Vector3d *pRa)
	{
		Eigen::Matrix4d mT = Eigen::Matrix4d::Identity();
		Vector3d eR(vR.x(), vR.y(), vR.z());
		mT.block(0, 0, 3, 3) = rotationMatrixFromXYZ(eR);
		mT(0, 3) = vT.x();
		mT(1, 3) = vT.y();
		mT(2, 3) = vT.z();

		NULL__(pRa, mT);

		eR = Vector3d(pRa->x(), pRa->y(), pRa->z());
		Eigen::Matrix3d mR = rotationMatrixFromAxisAngle(eR);
		Eigen::Matrix3d mTr = mT.block(0, 0, 3, 3);
		mT.block(0, 0, 3, 3) = mTr * mR;

		return mT;
	}

	Eigen::Matrix4d _PCtransform::createTranslationMatrix(const Vector3d &vT, const Vector4d &vQ, Vector3d *pRa)
	{
		Eigen::Matrix4d mT = Eigen::Matrix4d::Identity();
		Vector4d eQ(vQ.x(), vQ.y(), vQ.z(), vQ.w());
		mT.block(0, 0, 3, 3) = rotationMatrixFromQuaternion(eQ);
		mT(0, 3) = vT.x();
		mT(1, 3) = vT.y();
		mT(2, 3) = vT.z();

		NULL__(pRa, mT);

		Vector3d eR(pRa->x(), pRa->y(), pRa->z());
		Eigen::Matrix3d mR = rotationMatrixFromAxisAngle(eR);
		Eigen::Matrix3d mTr = mT.block(0, 0, 3, 3);
		mT.block(0, 0, 3, 3) = mR * mTr;

		return mT;
	}

	Eigen::Matrix4d _PCtransform::getTranslationMatrix(void)
	{
		return m_mT;
	}

	void _PCtransform::setTranslationMatrix(const Eigen::Matrix4d &mT)
	{
		m_mT = mT;
		m_A = m_mT;
	}

	Vector3d _PCtransform::getTranslation(void)
	{
		return m_vT;
	}

	Vector3d _PCtransform::getRotation(void)
	{
		return m_vR;
	}

	Vector4d _PCtransform::getQuaternion(void)
	{
		return m_vQ;
	}

    void _PCtransform::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        pC->addMsg("vT = (" + f2str(m_vT.x()) + "," + f2str(m_vT.y()) + ", " + f2str(m_vT.z()) + ")");
        pC->addMsg("vR = (" + f2str(m_vR.x()) + "," + f2str(m_vR.y()) + ", " + f2str(m_vR.z()) + ")");
        pC->addMsg("vQ = (" + f2str(m_vQ.x()) + "," + f2str(m_vQ.y()) + ", " + f2str(m_vQ.z()) + ", " + f2str(m_vQ.w()) + ")");
    }


}
