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

		Kiss *pKk = pK->child("threadK");
		IF_F(pKk->empty());
		m_pTk = new _Thread();
		if (!m_pTk->init(pKk))
		{
			DEL(m_pTk);
			return false;
		}

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

		return true;
	}

	bool _RopewayScanLivox::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTk);
		IF_F(!m_pTk->start(getUpdateKinematics, this));

		return true;
	}

	int _RopewayScanLivox::check(void)
	{
		NULL__(m_pNav, -1);
//		IF__(m_vpGB.empty(), -1);

		return this->_PCstream::check();
	}

	void _RopewayScanLivox::update(void)
	{
		m_pT->sleepT(0);

		// init
//		m_fProcess.set(pc_ScanReset);

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			// if (m_fProcess.b(pc_ScanReset, true))
			// 	scanReset();

			// if (m_fProcess.b(pc_ScanSet, true))
			// 	scanSet();

			// if (m_fProcess.b(pc_ScanStart, true))
			// 	scanStart();

			// if (m_fProcess.b(pc_ScanStop, true))
			// 	scanStop();

			// scanUpdate();

			// if (m_fProcess.b(pc_SavePC, true))
			// 	savePC();

			m_pT->autoFPSto();
		}
	}

	void _RopewayScanLivox::scanReset(void)
	{
		IF_(check() < 0);

		m_nP = 0;
		for (int i = 0; i < m_vPC.size(); i++)
		{
			PointCloud *pP = &m_vPC[i];
			pP->Clear();
		}
		m_vPC.clear();
	}

	void _RopewayScanLivox::scanStart(void)
	{
		IF_(check() < 0);
		m_bScanning = true;

//		_Livox *pPsrc = (_Livox *)m_vpGB[0];
//		pPsrc->startStream();
	}

	void _RopewayScanLivox::scanUpdate(void)
	{
		IF_(check() < 0);


		// Scanning
//		_Livox *pPsrc = (_Livox *)m_vpGB[0];
//		pPsrc->clear();

		//TODO: change to point number
//		sleep(m_scanSet.m_tWaitSec);
		scanTake();

	}

	void _RopewayScanLivox::scanTake(void)
	{
		IF_(check() < 0);

		// _PCstream *pPsrc = (_PCstream *)m_vpGB[0];
		// PointCloud pc;
		// pPsrc->getPC(&pc);
		// int nPnew = pc.points_.size();
		// IF_(nPnew <= 0);
		// int i;

		// // Add original
		// m_vPC.push_back(pc);
		// m_nP += pc.points_.size();

		// // Add voxel down for preview
		// PointCloud pcVd = *pc.VoxelDownSample(m_rVoxel);
		// int nPvd = pcVd.points_.size();
		// for (i = 0; i < nPvd; i++)
		// {
		// 	m_pPCprv->points_[m_iPprv] = pcVd.points_[i];
		// 	m_pPCprv->colors_[m_iPprv] = pcVd.colors_[i];
		// 	m_iPprv++;
		// 	if (m_iPprv >= m_nPresv)
		// 		break;
		// }

//		float rPorig = (float)m_nP / (float)m_nPmax;
//		float rPprv = (float)m_iPprv / (float)m_nPresv;
	}

	void _RopewayScanLivox::scanStop(void)
	{
		IF_(check() < 0);

		m_bScanning = false;

		// _Livox *pPsrc = (_Livox *)m_vpGB[0];
		// pPsrc->stopStream();
	}

	void _RopewayScanLivox::savePC(void)
	{
		IF_(check() < 0);
		if (m_nP <= 0)
		{
			return;
		}

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

		string msg;
		if (nSave > m_vPC.size())
		{
			msg = "Saved to: " + m_dir;
		}
		else
		{
			msg = "Failed to save: " + m_dir;
		}
	}

	void _RopewayScanLivox::updateKinematics(void)
	{
		while (m_pTk->bRun())
		{
			m_pTk->autoFPSfrom();

			updateSlam();

			m_pTk->autoFPSto();
		}
	}

	void _RopewayScanLivox::updateSlam(void)
	{
		IF_(check() < 0);
		IF_(!m_pNav->bOpen());

		auto mT = m_pNav->mT();
		// for (int i = 0; i < m_vpGB.size(); i++)
		// {
		// 	_GeometryBase *pP = m_vpGB[i];
		// 	pP->setTranslation(mT.cast<double>());
		// }
	}


}
