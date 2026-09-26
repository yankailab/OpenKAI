/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_Canvas.h"

namespace kai
{

	_Canvas::_Canvas()
	{
		m_vArea = Vector2f(-FLT_MAX, FLT_MAX);
		m_vW = Vector2f(-FLT_MAX, FLT_MAX);
		m_vH = Vector2f(-FLT_MAX, FLT_MAX);

		m_vRoi = Vector4f(0.0, 0.0, 1.0, 1.0);
		m_vClassRange = Vector2i(-INT_MAX, INT_MAX);

		clear();
	}

	_Canvas::~_Canvas()
	{
	}

	bool _Canvas::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		// general
		jKv(j, "minConfidence", m_minConfidence);
		jKv<float>(j, "vArea", m_vArea);
		jKv<float>(j, "vW", m_vW);
		jKv<float>(j, "vH", m_vH);
		const auto roi = j.value("vRoi", vector<float>{0, 0, 1, 1});
		for (size_t i = 0; i < std::min(roi.size(), size_t{4}); ++i)
			m_vRoi[i] = roi[i];
		jKv<int>(j, "vClassRange", m_vClassRange);

		// draw
		jKv(j, "bDrawText", m_bDrawText);
		jKv(j, "bDrawPos", m_bDrawPos);
		jKv(j, "bDrawBB", m_bDrawBB);

		// buffer
		int nB = 16;
		jKv(j, "nBuf", nB);
		m_sO.get()->init(nB);
		m_sO.next()->init(nB);
		clear();

		return true;
	}

	void _Canvas::clear(void)
	{
		m_sO.get()->clear();
		m_sO.next()->clear();
	}

	void _Canvas::swap(void)
	{
		m_sO.swap();
		m_sO.next()->clear();
	}

	bool _Canvas::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Canvas::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
		}
	}

	_Object *_Canvas::add(_Object &o)
	{
		IF__((o.getDimArea() < m_vArea.x() || o.getDimArea() >= m_vArea.y()), nullptr);
		IF__((o.getDim().x() < m_vW.x() || o.getDim().x() >= m_vW.y()), nullptr);
		IF__((o.getDim().y() < m_vH.x() || o.getDim().y() >= m_vH.y()), nullptr);
		IF__((o.getTopClass() < m_vClassRange.x() || o.getTopClass() >= m_vClassRange.y()), nullptr);

		Vector3f p = o.getPos();
		IF__(p.x() < m_vRoi.x(), nullptr);
		IF__(p.x() > m_vRoi.z(), nullptr);
		IF__(p.y() < m_vRoi.y(), nullptr);
		IF__(p.y() > m_vRoi.w(), nullptr);

		return m_sO.next()->add(o);
	}

	_Object *_Canvas::get(int i)
	{
		return m_sO.get()->get(i);
	}

	int _Canvas::size(void)
	{
		return m_sO.get()->size();
	}

	void _Canvas::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		((_Console *)pConsole)->addMsg("nObj=" + i2str(m_sO.get()->size()), 1);
	}

	void _Canvas::draw(void *pMat)
	{
#ifdef USE_OPENCV
		NULL_(pMat);
		this->_ModuleBase::draw(pMat);
		IF_(!check());

		Mat *pM = static_cast<Mat *>(pMat);
		IF_(pM->empty());

		_Object *pO;
		Scalar oCol;
		Scalar bCol = Scalar(100, 100, 100);
		int col;
		int colStep = 20;
		int i = 0;

		while ((pO = get(i++)) != NULL)
		{
			if (pO->getType() == obj_tag)
			{
				Vector3f vP = pO->getPos();
				Point pCenter = Point(vP.x() * pM->cols, vP.y() * pM->rows);
				int r = pO->getDim().w(); // * pM->cols;

				circle(*pM, pCenter, r, Scalar(255, 255, 0), 2);

				putText(*pM, "iTag=" + i2str(pO->getTopClass()) + ", angle=" + i2str(pO->getAttitude().x()),
						pCenter,
						FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 0);

				double rad = -pO->getAttitude().x() * DEG_2_RAD;
				Point pD = Point(r * sin(rad), r * cos(rad));
				line(*pM, pCenter + pD, pCenter - pD, Scalar(0, 0, 255), 2);
			}
			else
			{
				int iClass = pO->getTopClass();

				col = colStep * iClass;
				oCol = Scalar((col + 85) % 255, (col + 170) % 255, col) + bCol;

				// bb
				Rect r = bb2Rect<Vector4f>(pO->getBB2D(pM->cols, pM->rows));
				rectangle(*pM, r, oCol, 1);

				// position
				if (m_bDrawPos)
				{
					putText(*pM, f2str(pO->getPos().z()),
							Point(r.x + 15, r.y + 25),
							FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
				}

				// text
				if (m_bDrawText)
				{
					string oName = string(pO->getText());
					if (oName.length() > 0)
					{
						putText(*pM, oName,
								Point(r.x + 15, r.y + 50),
								FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
					}
				}

				// BB
				if (m_bDrawBB)
				{
					string strBB = "(" + i2str(r.x) + ", " + i2str(r.y) + ", " + i2str(r.width) + "," + i2str(r.height) + ")";
					putText(*pM, strBB,
							Point(r.x + 15, r.y + 50),
							FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
				}
			}
		}

		// roi
		Rect roi = bb2Rect(bbScale(m_vRoi, pM->cols, pM->rows));
		rectangle(*pM, roi, Scalar(0, 255, 255), 1);
#endif
	}

}
