#include "_GeoFence.h"

namespace kai
{
	_GeoFence::_GeoFence()
	{
		m_vP.setZero();
		m_vPnext.setZero();
	}

	_GeoFence::~_GeoFence()
	{
	}

	bool _GeoFence::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "type", m_type);
		jKv(j, "estD", m_estD);
		jKv<double>(j, "vP", m_vP);
		jKv(j, "vPolygon", m_vPolygon);

		return true;
	}

	bool _GeoFence::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_JSONbase", n);
		m_pJb = (_JSONbase *)(m_pM->findModule(n));

		return true;
	}

	bool _GeoFence::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		j["type"] = m_type;
		j["estD"] = m_estD;
		j["vP"] = {m_vP[0], m_vP[1]};
		j["vPolygon"] = m_vPolygon;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _GeoFence::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _GeoFence::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateFencePolygon();
			sendFence();
		}
	}

	_GeoFence_TYPE _GeoFence::getType(void)
	{
		return m_type;
	}

	void _GeoFence::sendFence(void)
	{
		NULL_(m_pJb);

		json j = json::object();
		j["cmd"] = "geoFence";
		j["vP"] = {m_vP.x(), m_vP.y()}; // lat lon
		j["vPnext"] = {m_vPnext.x(), m_vPnext.y()};
		j["bBreach"] = m_bBreach;
		m_pJb->sendJson(j);
	}

	void _GeoFence::setPosHdg(const Vector2d &vP, float hdgDeg)
	{
		m_vP = vP;
		m_hdg = hdgDeg;
	}

	void _GeoFence::setPolygon(const vector<vector<double>> &vvCoord)
	{
		m_vPolygon = vvCoord;
	}

	bool _GeoFence::bBreach(void)
	{
		return m_bBreach;
	}

	void _GeoFence::getP(Vector2d *pP, Vector2d *pPnext)
	{
		if (pP)
		{
			*pP = m_vP;
		}

		if (pPnext)
		{
			*pPnext = m_vPnext;
		}
	}

	void _GeoFence::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("bBreach: " + i2str(m_bBreach));
		pC->addMsg("nPolygonVertices:" + i2str(m_vPolygon.size()));
		pC->addMsg("vP: (" + lf2str(m_vP.x(), 7) + ", " + lf2str(m_vP.y(), 7) + ")");
		pC->addMsg("hdg: " + f2str(m_hdg));
	}

	void _GeoFence::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		if (cmd == "setGeoFence")
		{
			string type;
			jKv(j, "type", type);

			if (type == "polygon")
			{
				jKv(j, "vPolygon", m_vPolygon);
				setPolygon(m_vPolygon);
			}
			else if (type == "circle")
			{
			}

			const bool bSuccess = saveConfig(true);

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "setGeoFence";
			jr["bSuccess"] = bSuccess;
			pJb->sendJson(jr);

			m_pJb = pJb;
		}
		if (cmd == "loadGeoFence")
		{
			NULL_(pJb);

			json jr = json::object();
			jr["cmd"] = "loadGeoFence";
			jr["bSuccess"] = true;
			jr["type"] = m_type == _GeoFence_polygon ? "polygon" : "circle";
			jr["vPolygon"] = m_vPolygon;
			pJb->sendJson(jr);

			m_pJb = pJb;
		}

	}

	constexpr double kPi = 3.14159265358979323846;
	constexpr double kDegToRad = kPi / 180.0;
	constexpr double kRadToDeg = 180.0 / kPi;
	constexpr double kEarthRadiusM = 6378137.0;
	constexpr double kEps = 1e-9;

	inline double normalizeHeadingDeg(double deg)
	{
		while (deg < 0.0)
			deg += 360.0;
		while (deg >= 360.0)
			deg -= 360.0;
		return deg;
	}

	inline double dot2(const Vector2d &a, const Vector2d &b)
	{
		return a.dot(b);
	}

	inline double cross2(const Vector2d &a, const Vector2d &b)
	{
		return a.x() * b.y() - a.y() * b.x();
	}

	inline double norm2(const Vector2d &a)
	{
		return a.norm();
	}

	inline Vector2d normalize2(const Vector2d &a)
	{
		const double n = norm2(a);
		if (n < kEps)
			return {0.0, 0.0};
		return a / n;
	}

	// Convert geo point (lat, lon) to local ENU-like planar coordinates in meters:
	// local.x() = East, local.y() = North
	// Reference origin = refGeo
	inline Vector2d geoToLocalMeters(const Vector2d &geo, const Vector2d &refGeo)
	{
		const double refLatRad = refGeo.x() * kDegToRad;
		const double dLatRad = (geo.x() - refGeo.x()) * kDegToRad;
		const double dLonRad = (geo.y() - refGeo.y()) * kDegToRad;

		const double north = kEarthRadiusM * dLatRad;
		const double east = kEarthRadiusM * std::cos(refLatRad) * dLonRad;

		return {east, north};
	}

	inline Vector2d localMetersToGeo(const Vector2d &local, const Vector2d &refGeo)
	{
		const double refLatRad = refGeo.x() * kDegToRad;

		const double dLatRad = local.y() / kEarthRadiusM;
		const double dLonRad = local.x() / (kEarthRadiusM * std::cos(refLatRad));

		const double lat = refGeo.x() + dLatRad * kRadToDeg;
		const double lon = refGeo.y() + dLonRad * kRadToDeg;

		return {lat, lon};
	}

	inline bool pointOnSegment(const Vector2d &p, const Vector2d &a, const Vector2d &b)
	{
		const Vector2d ab = b - a;
		const Vector2d ap = p - a;
		const double area2 = std::fabs(cross2(ab, ap));
		IF_F(area2 > 1e-7);

		const double d = dot2(ap, ab);
		IF_F(d < -1e-7);

		const double ab2 = dot2(ab, ab);
		IF_F(d > ab2 + 1e-7);

		return true;
	}

	// Ray casting. Boundary counts as inside.
	bool pointInPolygonOrOnEdge(const vector<Vector2d> &poly, const Vector2d &p)
	{
		const size_t n = poly.size();
		IF_F(n < 3);

		bool inside = false;
		for (size_t i = 0, j = n - 1; i < n; j = i++)
		{
			const Vector2d &a = poly[j];
			const Vector2d &b = poly[i];

			IF__(pointOnSegment(p, a, b), true);

			const bool intersect =
				((a.y() > p.y()) != (b.y() > p.y())) &&
				(p.x() < (b.x() - a.x()) * (p.y() - a.y()) / ((b.y() - a.y()) + 1e-30) + a.x());

			if (intersect)
				inside = !inside;
		}
		return inside;
	}

	// Segment-segment intersection:
	// p + t*r intersects a + u*s
	// Returns true if intersecting, and outputs t/u/intersection.
	bool segmentIntersect(
		const Vector2d &p0,
		const Vector2d &p1,
		const Vector2d &a,
		const Vector2d &b,
		double &outT,
		double &outU,
		Vector2d &outPt)
	{
		const Vector2d r = p1 - p0;
		const Vector2d s = b - a;
		const double denom = cross2(r, s);
		const Vector2d ap = a - p0;

		// Parallel or collinear: ignore for reflection purposes here.
		IF_F(std::fabs(denom) < kEps);

		const double t = cross2(ap, s) / denom;
		const double u = cross2(ap, r) / denom;

		if (t >= -1e-9 && t <= 1.0 + 1e-9 &&
			u >= -1e-9 && u <= 1.0 + 1e-9)
		{
			outT = t;
			outU = u;
			outPt = p0 + r * t;
			return true;
		}

		return false;
	}

	// Reflect direction d across the infinite line whose tangent is edgeUnit.
	inline Vector2d reflectAcrossEdgeTangent(const Vector2d &d, const Vector2d &edgeUnit)
	{
		// Reflection across line: r = 2*proj_line(d) - d
		const double proj = dot2(d, edgeUnit);
		return edgeUnit * (2.0 * proj) - d;
	}

	// Convert local motion vector to navigation heading:
	// x = East, y = North, heading 0=N, 90=E
	inline double vectorToHeadingDeg(const Vector2d &v)
	{
		const double hdgRad = std::atan2(v.x(), v.y()); // atan2(East, North)
		return normalizeHeadingDeg(hdgRad * kRadToDeg);
	}

	void _GeoFence::updateFencePolygon(void)
	{
		IF_(m_vPolygon.size() < 3); // invalid polygon

		// Build local polygon around current position.
		vector<Vector2d> polyLocal;
		polyLocal.reserve(m_vPolygon.size());
		for (const auto &g : m_vPolygon)
			polyLocal.push_back(geoToLocalMeters(Vector2d(g[0], g[1]), m_vP));

		// Current robot position is origin in local frame.
		const Vector2d p0{0.0, 0.0};

		// Heading convention: 0=N, 90=E
		const double hdgRad = static_cast<double>(m_hdg) * kDegToRad;
		const Vector2d moveVec{
			static_cast<double>(m_estD) * std::sin(hdgRad), // East
			static_cast<double>(m_estD) * std::cos(hdgRad)	// North
		};

		const Vector2d p1 = p0 + moveVec;

		// Compute the new geo position
		m_vPnext = localMetersToGeo(p1, m_vP);

		// If new point is still inside (or on edge), no breach.
		if (pointInPolygonOrOnEdge(polyLocal, p1))
		{
			m_bBreach = false;
			m_rAngle = 0;
			return;
		}

		// Breach
		m_bBreach = true;

		// Compute reflection heading from the crossed edge.
		double bestT = std::numeric_limits<double>::infinity();
		bool foundEdge = false;
		Vector2d bestA = Vector2d::Zero(), bestB = Vector2d::Zero(), ip = Vector2d::Zero();

		const size_t n = polyLocal.size();
		for (size_t i = 0; i < n; ++i)
		{
			const Vector2d &a = polyLocal[i];
			const Vector2d &b = polyLocal[(i + 1) % n];

			double t = 0.0, u = 0.0;
			Vector2d hit = Vector2d::Zero();
			if (segmentIntersect(p0, p1, a, b, t, u, hit))
			{
				// Ignore the trivial start point touch if already exactly on boundary.
				if (t < 1e-8)
					continue;

				if (t < bestT)
				{
					bestT = t;
					bestA = a;
					bestB = b;
					ip = hit;
					foundEdge = true;
				}
			}
		}

		if (foundEdge)
		{
			const Vector2d edge = bestB - bestA;
			const Vector2d edgeUnit = normalize2(edge);
			const Vector2d dirUnit = normalize2(moveVec);

			if (norm2(edgeUnit) > kEps && norm2(dirUnit) > kEps)
			{
				const Vector2d refl = reflectAcrossEdgeTangent(dirUnit, edgeUnit);
				m_rAngle = static_cast<float>(vectorToHeadingDeg(refl));
			}
		}
	}

}
