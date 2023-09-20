/*
 * _RopewayScanLivox.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_RopewayScanLivox.h"

namespace kai
{
	_RopewayScanLivox::_RopewayScanLivox()
	{
		m_pNav = NULL;
		m_pTk = NULL;

		m_nP = 0;
		m_nPmax = INT_MAX;
		m_rVoxel = 0.1;
		m_fProcess.clearAll();
		m_baseDir = "";
		m_dir = "";
		
		m_tWait = 5;
	}

	_RopewayScanLivox::~_RopewayScanLivox()
	{
		DEL(m_pTk);
	}

	bool _RopewayScanLivox::init(void *pKiss)
	{
		IF_F(!this->_PCstream::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("rVoxel", &m_rVoxel);
		pK->v("nPmax", &m_nPmax);
		pK->v("baseDir", &m_baseDir);
		pK->v("tWait", &m_tWait);

		Kiss *pKk = pK->child("threadK");
		IF_F(pKk->empty());
		m_pTk = new _Thread();
		if (!m_pTk->init(pKk))
		{
			DEL(m_pTk);
			return false;
		}

		m_fProcess.set(rws_lv_reset);

		return true;
	}

	bool _RopewayScanLivox::link(void)
	{
		IF_F(!this->_PCstream::link());
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

	bool _RopewayScanLivox::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTk);
		IF_F(!m_pTk->start(getUpdateK, this));

		return true;
	}

	int _RopewayScanLivox::check(void)
	{
		NULL__(m_pNav, -1);
		IF__(m_vpGB.empty(), -1);

		return this->_PCstream::check();
	}

	void _RopewayScanLivox::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (m_fProcess.b(rws_lv_reset, true))
				scanReset();

			if (m_fProcess.b(rws_lv_take, true))
				scanTake();

			if (m_fProcess.b(rws_lv_save, true))
				savePC();

			m_pT->autoFPSto();
		}
	}

	void _RopewayScanLivox::scanReset(void)
	{
		IF_(check() < 0);

		_Livox *pPsrc = (_Livox *)m_vpGB[0];
		pPsrc->clear();

		m_nP = 0;
		for (int i = 0; i < m_vPC.size(); i++)
		{
			PointCloud *pP = &m_vPC[i];
			pP->Clear();
		}
		m_vPC.clear();

		m_rB = 0.0;
	}

	void _RopewayScanLivox::scanStart(void)
	{
		IF_(check() < 0);

		_Livox *pPsrc = (_Livox *)m_vpGB[0];
		pPsrc->startStream();
	}

	void _RopewayScanLivox::scanStop(void)
	{
		IF_(check() < 0);

		_Livox *pPsrc = (_Livox *)m_vpGB[0];
		pPsrc->stopStream();
	}

	void _RopewayScanLivox::scanTake(void)
	{
		IF_(check() < 0);

		_Livox *pPsrc = (_Livox *)m_vpGB[0];
		pPsrc->clear();
		sleep(m_tWait);

		PointCloud pc;
		pPsrc->getPC(&pc);
		int nPnew = pc.points_.size();
		IF_(nPnew <= 0);

		// Add original
		m_vPC.push_back(pc);
		m_nP += nPnew;

		m_rB = (float)m_nP / (float)m_nPmax;
	}

	void _RopewayScanLivox::savePC(void)
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

	bool _RopewayScanLivox::bBusy(void)
	{
		return m_fProcess.bClear();
	}

	void _RopewayScanLivox::resetScan(void)
	{
		m_fProcess.set(rws_lv_reset);
	}

	void _RopewayScanLivox::startScan(void)
	{
		m_fProcess.set(rws_lv_start);
	}

	void _RopewayScanLivox::stopScan(void)
	{
		m_fProcess.set(rws_lv_stop);
	}

	void _RopewayScanLivox::takeScan(void)
	{
		m_fProcess.set(rws_lv_take);
	}

	void _RopewayScanLivox::save(void)
	{
		m_fProcess.set(rws_lv_save);
	}

	void _RopewayScanLivox::updateK(void)
	{
		while (m_pTk->bRun())
		{
			m_pTk->autoFPSfrom();

			updateAttitude();

			m_pTk->autoFPSto();
		}
	}

	void _RopewayScanLivox::updateAttitude(void)
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
