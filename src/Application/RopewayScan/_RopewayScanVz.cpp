/*
 * _RopewayScanVz.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_RopewayScanVz.h"

namespace kai
{
	_RopewayScanVz::_RopewayScanVz()
	{
		m_pNav = NULL;
		m_pTk = NULL;

		m_nP = 0;
		m_nPmax = INT_MAX;
		m_rVoxel = 0.1;
		m_fProcess.clearAll();
		m_baseDir = "";
		m_dir = "";
	}

	_RopewayScanVz::~_RopewayScanVz()
	{
		DEL(m_pTk);
	}

	bool _RopewayScanVz::init(void *pKiss)
	{
		IF_F(!this->_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("rVoxel", &m_rVoxel);
		pK->v("nPmax", &m_nPmax);
		pK->v("baseDir", &m_baseDir);

		Kiss *pKk = pK->child("threadK");
		IF_F(pKk->empty());
		m_pTk = new _Thread();
		if (!m_pTk->init(pKk))
		{
			DEL(m_pTk);
			return false;
		}

		m_fProcess.set(rws_vz_reset);

		return true;
	}

	bool _RopewayScanVz::link(void)
	{
		IF_F(!this->_PCframe::link());
		IF_F(!m_pTk->link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n = "";
		F_ERROR_F(pK->v("_NavBase", &n));
		m_pNav = (_NavBase *)(pK->getInst(n));
		NULL_Fl(m_pNav, n + ": not found");

		vector<string> vGB;
		pK->a("vGeometryBase", &vGB);
		for (string p : vGB)
		{
			_GeometryBase *pGB = (_GeometryBase *)(pK->getInst(p));
			IF_CONT(!pGB);

			m_vpGB.push_back(pGB);
		}

		return true;
	}

	bool _RopewayScanVz::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTk);
		IF_F(!m_pTk->start(getUpdateK, this));

		return true;
	}

	int _RopewayScanVz::check(void)
	{
		NULL__(m_pNav, -1);
		IF__(m_vpGB.empty(), -1);

		return this->_PCframe::check();
	}

	void _RopewayScanVz::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (m_fProcess.b(rws_vz_reset, true))
				scanReset();

			if (m_fProcess.b(rws_vz_take, true))
				scanTake();

			if (m_fProcess.b(rws_vz_save, true))
				savePC();

			m_pT->autoFPSto();
		}
	}

	void _RopewayScanVz::scanReset(void)
	{
		m_nP = 0;
		for (int i = 0; i < m_vPC.size(); i++)
		{
			PointCloud *pP = &m_vPC[i];
			pP->Clear();
		}
		m_vPC.clear();

		m_rB = 0.0;
	}

	void _RopewayScanVz::scanTake(void)
	{
		IF_(check() < 0);

		_PCframe *pPsrc = (_PCframe *)m_vpGB[0];
		PointCloud pc;
		pPsrc->getPC(&pc);
		int nPnew = pc.points_.size();
		IF_(nPnew <= 0);

		// Add original
		m_vPC.push_back(pc);
		m_nP += nPnew;

		m_rB = (float)m_nP / (float)m_nPmax;
	}

	void _RopewayScanVz::savePC(void)
	{
		IF_(check() < 0);
		IF_(m_nP <= 0);

		// Make new folder
		m_dir = m_baseDir + tFormat() + "/";
		string cmd = "mkdir " + m_dir;
		system(cmd.c_str());

		io::WritePointCloudOption par;
		par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
		par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

		PointCloud pcMerge;
		int nSave = 0;
		for (int i = 0; i < m_vPC.size(); i++)
		{
			PointCloud *pP = &m_vPC[i];
			nSave += (io::WritePointCloudToPLY(m_dir + i2str(i) + ".ply",
											   *pP,
											   par)) ? 1 : 0;

			pcMerge += *pP;
		}

		nSave += (io::WritePointCloudToPLY(m_dir + "_merged.ply",
										   pcMerge,
										   par)) ? 1 : 0;
	}


	bool _RopewayScanVz::bBusy(void)
	{
		return m_fProcess.bClear();
	}

	void _RopewayScanVz::reset(void)
	{
		m_fProcess.set(rws_vz_reset);
	}

	void _RopewayScanVz::take(void)
	{
		m_fProcess.set(rws_vz_take);
	}

	void _RopewayScanVz::save(void)
	{
		m_fProcess.set(rws_vz_save);
	}

	float _RopewayScanVz::getBufferCap(void)
	{
		return m_rB;
	}

	void _RopewayScanVz::updateK(void)
	{
		while (m_pTk->bRun())
		{
			m_pTk->autoFPSfrom();

			updateAttitude();

			m_pTk->autoFPSto();
		}
	}

	void _RopewayScanVz::updateAttitude(void)
	{
		IF_(check() < 0);
		IF_(!m_pNav->bOpen());

		auto mT = m_pNav->mT();
		for (int i = 0; i < m_vpGB.size(); i++)
		{
			_GeometryBase *pP = m_vpGB[i];
			pP->setTranslation(mT.cast<double>());
		}
	}

}
