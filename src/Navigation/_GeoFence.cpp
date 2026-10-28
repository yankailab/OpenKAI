#include "_GeoFence.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace kai
{
	_GeoFence::_GeoFence()
	{
		m_vP.setZero();
		m_vPnext.setZero();
	}

	_GeoFence::~_GeoFence()
	{
		if (m_pT)
			m_pT->join();
	}

	bool _GeoFence::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		std::lock_guard<std::mutex> lock(m_fenceMutex);
		jKv(j, "type", m_type);
		jKv(j, "estD", m_estD);
		jKv<double>(j, "vP", m_vP);
		jKv(j, "vPolygon", m_vPolygon);
		evaluateFencePolygon();

		return true;
	}

	bool _GeoFence::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_JSONbase", n);
		std::lock_guard<std::mutex> lock(m_fenceMutex);
		m_pJb = (_JSONbase *)(pM->findModule(n));

		return true;
	}

	bool _GeoFence::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		{
			std::lock_guard<std::mutex> lock(m_fenceMutex);
			json &j = *m_pJ;
			j["type"] = m_type;
			j["estD"] = m_estD;
			j["vP"] = {m_vP[0], m_vP[1]};
			j["vPolygon"] = m_vPolygon;
		}

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
		std::lock_guard<std::mutex> lock(m_fenceMutex);
		return m_type;
	}

	void _GeoFence::sendFence(void)
	{
		json j = json::object();
		_JSONbase *pJb;
		{
			std::lock_guard<std::mutex> lock(m_fenceMutex);
			pJb = m_pJb;
			NULL_(pJb);
			j["cmd"] = "geoFence";
			j["vP"] = {m_vP.x(), m_vP.y()}; // lat lon
			j["vPnext"] = {m_vPnext.x(), m_vPnext.y()};
			j["bValid"] = m_status.bValid;
			j["bBreach"] = m_status.bCurrentBreach || m_status.bPredictedBreach;
			j["bCurrentBreach"] = m_status.bCurrentBreach;
			j["bPredictedBreach"] = m_status.bPredictedBreach;
			j["bTurnRequired"] = m_status.bTurnRequired;
			j["attackDeg"] = m_status.attackDeg;
			j["reflectedHdgDeg"] = m_status.reflectedHdgDeg;
			j["turnDeg"] = m_status.turnDeg;
		}
		pJb->sendJson(j);
	}

	void _GeoFence::setPosHdg(const Vector2d &vP, float hdgDeg)
	{
		std::lock_guard<std::mutex> lock(m_fenceMutex);
		m_vP = vP;
		m_hdg = hdgDeg;
		evaluateFencePolygon();
	}

	void _GeoFence::setPolygon(const vector<vector<double>> &vvCoord)
	{
		std::lock_guard<std::mutex> lock(m_fenceMutex);
		m_vPolygon = vvCoord;
		m_type = _GeoFence_polygon;
		evaluateFencePolygon();
	}

	bool _GeoFence::bBreach(void)
	{
		const _GeoFence_STATUS status = getStatus();
		return status.bCurrentBreach || status.bPredictedBreach;
	}

	_GeoFence_STATUS _GeoFence::getStatus(void)
	{
		std::lock_guard<std::mutex> lock(m_fenceMutex);
		return m_status;
	}

	void _GeoFence::getP(Vector2d *pP, Vector2d *pPnext)
	{
		std::lock_guard<std::mutex> lock(m_fenceMutex);
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
		std::lock_guard<std::mutex> lock(m_fenceMutex);
		pC->addMsg("bValid: " + i2str(m_status.bValid));
		pC->addMsg("bBreach: " + i2str(m_status.bCurrentBreach || m_status.bPredictedBreach));
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
				vector<vector<double>> polygon;
				try
				{
					jKv(j, "vPolygon", polygon);
				}
				catch (const json::exception &)
				{
					polygon.clear();
				}
				setPolygon(polygon);
			}

			const bool bSuccess = type == "polygon" && getStatus().bValid && saveConfig(true);

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "setGeoFence";
			jr["bSuccess"] = bSuccess;
			pJb->sendJson(jr);

			std::lock_guard<std::mutex> lock(m_fenceMutex);
			m_pJb = pJb;
		}
		if (cmd == "loadGeoFence")
		{
			NULL_(pJb);

			json jr = json::object();
			jr["cmd"] = "loadGeoFence";
			jr["bSuccess"] = true;
			{
				std::lock_guard<std::mutex> lock(m_fenceMutex);
				jr["type"] = m_type == _GeoFence_polygon ? "polygon" : "circle";
				jr["vPolygon"] = m_vPolygon;
				m_pJb = pJb;
			}
			pJb->sendJson(jr);
		}

	}

	namespace
	{
		constexpr double kPi = 3.14159265358979323846;
		constexpr double kDegToRad = kPi / 180.0;
		constexpr double kRadToDeg = 180.0 / kPi;
		constexpr double kEarthRadiusM = 6378137.0;
		constexpr double kEps = 1e-9;
		constexpr double kBoundaryEpsM = 1e-6;

		double normalizeHeadingDeg(double deg)
		{
			deg = std::fmod(deg, 360.0);
			return deg < 0 ? deg + 360.0 : deg;
		}

		double signedAngleDeg(double deg)
		{
			deg = normalizeHeadingDeg(deg);
			return deg > 180.0 ? deg - 360.0 : deg;
		}

		double cross2(const Vector2d &a, const Vector2d &b)
		{
			return a.x() * b.y() - a.y() * b.x();
		}

		bool validGeo(const Vector2d &p)
		{
			return p.allFinite() && std::abs(p.x()) < 89.999999 && std::abs(p.y()) <= 180.0;
		}

		// Local east/north meters, sufficient for a lawnmower's operating area.
		Vector2d geoToLocalMeters(const Vector2d &geo, const Vector2d &refGeo)
		{
			return {kEarthRadiusM * std::cos(refGeo.x() * kDegToRad) *
				(geo.y() - refGeo.y()) * kDegToRad,
				kEarthRadiusM * (geo.x() - refGeo.x()) * kDegToRad};
		}

		Vector2d localMetersToGeo(const Vector2d &local, const Vector2d &refGeo)
		{
			return {refGeo.x() + local.y() / kEarthRadiusM * kRadToDeg,
				refGeo.y() + local.x() / (kEarthRadiusM * std::cos(refGeo.x() * kDegToRad)) * kRadToDeg};
		}

		Vector2d closestOnSegment(const Vector2d &p, const Vector2d &a, const Vector2d &b)
		{
			const Vector2d edge = b - a;
			const double length2 = edge.squaredNorm();
			if (length2 < kEps * kEps)
				return a;
			const double t = std::max(0.0, std::min(1.0, (p - a).dot(edge) / length2));
			return a + edge * t;
		}

		// Boundary counts as inside. Degenerate edges are points, never infinite boundaries.
		bool pointInPolygonOrOnEdge(const vector<Vector2d> &poly, const Vector2d &p)
		{
			bool inside = false;
			for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++)
			{
				const Vector2d &a = poly[j], &b = poly[i];
				if ((p - closestOnSegment(p, a, b)).norm() <= kBoundaryEpsM)
					return true;
				if (((a.y() > p.y()) != (b.y() > p.y())) &&
					p.x() < (b.x() - a.x()) * (p.y() - a.y()) / (b.y() - a.y()) + a.x())
					inside = !inside;
			}
			return inside;
		}

		double distanceToPolygon(const vector<Vector2d> &poly, const Vector2d &p,
			Vector2d *pClosest = nullptr, size_t *pEdge = nullptr)
		{
			double best = std::numeric_limits<double>::infinity();
			for (size_t i = 0; i < poly.size(); ++i)
			{
				const Vector2d q = closestOnSegment(p, poly[i], poly[(i + 1) % poly.size()]);
				const double distance = (p - q).norm();
				if (distance < best)
				{
					best = distance;
					if (pClosest) *pClosest = q;
					if (pEdge) *pEdge = i;
				}
			}
			return best;
		}

		double vectorToHeadingDeg(const Vector2d &v)
		{
			return normalizeHeadingDeg(std::atan2(v.x(), v.y()) * kRadToDeg);
		}

		struct FenceHit
		{
			double t;
			Vector2d outward;
		};
	}

	void _GeoFence::updateFencePolygon(void)
	{
		std::lock_guard<std::mutex> lock(m_fenceMutex);
		evaluateFencePolygon();
	}

	void _GeoFence::evaluateFencePolygon(void)
	{
		// Invalid/removed fences and bad telemetry must never retain an earlier turn command.
		m_status = _GeoFence_STATUS();
		m_vPnext = m_vP.allFinite() ? m_vP : Vector2d::Zero();
		if (m_type != _GeoFence_polygon || !validGeo(m_vP) || !std::isfinite(m_hdg) ||
			!std::isfinite(m_estD) || m_estD < 0 || m_vPolygon.size() < 3)
			return;

		vector<Vector2d> poly;
		poly.reserve(m_vPolygon.size());
		for (const auto &geo : m_vPolygon)
		{
			if (geo.size() != 2 || !validGeo(Vector2d(geo[0], geo[1])))
				return;
			const Vector2d local = geoToLocalMeters(Vector2d(geo[0], geo[1]), m_vP);
			if (poly.empty() || (local - poly.back()).norm() > kBoundaryEpsM)
				poly.push_back(local);
		}
		if (poly.size() > 1 && (poly.front() - poly.back()).norm() <= kBoundaryEpsM)
			poly.pop_back();
		if (poly.size() < 3)
			return;

		double twiceArea = 0;
		for (size_t i = 0; i < poly.size(); ++i)
			twiceArea += cross2(poly[i], poly[(i + 1) % poly.size()]);
		if (std::abs(twiceArea) < kBoundaryEpsM * kBoundaryEpsM)
			return;
		const double winding = twiceArea > 0 ? 1.0 : -1.0;

		const double heading = normalizeHeadingDeg(m_hdg);
		const double hdgRad = heading * kDegToRad;
		const Vector2d direction(std::sin(hdgRad), std::cos(hdgRad));
		const Vector2d p0 = Vector2d::Zero();
		const Vector2d p1 = direction * m_estD;
		m_vPnext = localMetersToGeo(p1, m_vP);
		m_status.bValid = true;
		m_status.bCurrentBreach = !pointInPolygonOrOnEdge(poly, p0);
		m_status.bPredictedBreach = !pointInPolygonOrOnEdge(poly, p1);
		m_status.reflectedHdgDeg = static_cast<float>(heading);

		// Inspect crossings as well as the endpoint: a concave fence may be exited and
		// re-entered within one prediction segment. Keep both edges at a corner hit.
		vector<FenceHit> hits;
		for (size_t i = 0; i < poly.size(); ++i)
		{
			const Vector2d &a = poly[i];
			const Vector2d edge = poly[(i + 1) % poly.size()] - a;
			const double denom = cross2(p1, edge);
			if (std::abs(denom) < kEps)
				continue;
			const double t = cross2(a, edge) / denom;
			const double u = cross2(a, p1) / denom;
			if (t < -kEps || t > 1.0 + kEps || u < -kEps || u > 1.0 + kEps)
				continue;
			const Vector2d outward = winding * Vector2d(edge.y(), -edge.x()).normalized();
			hits.push_back({std::max(0.0, std::min(1.0, t)), outward});
		}
		std::sort(hits.begin(), hits.end(), [](const FenceHit &a, const FenceHit &b) { return a.t < b.t; });

		Vector2d outward = Vector2d::Zero();
		bool exits = false;
		for (size_t i = 0; i < hits.size();)
		{
			size_t next = i + 1;
			while (next < hits.size() && std::abs(hits[next].t - hits[i].t) < kEps)
				++next;
			const double end = next < hits.size() ? hits[next].t : 1.0;
			const Vector2d sample = p1 * ((hits[i].t + end) * 0.5);
			if (end - hits[i].t > kEps && !pointInPolygonOrOnEdge(poly, sample))
			{
				for (size_t j = i; j < next; ++j)
					if (direction.dot(hits[j].outward) > kEps)
						outward += hits[j].outward;
				if (outward.norm() > kEps)
				{
					exits = true;
					outward.normalize();
					break;
				}
			}
			i = next;
		}
		m_status.bPredictedBreach = m_status.bPredictedBreach || exits;

		Vector2d reflected = direction;
		if (exits)
		{
			m_status.bTurnRequired = true;
			reflected = direction - 2.0 * direction.dot(outward) * outward;
		}
		else if (m_status.bCurrentBreach)
		{
			// A short prediction may remain outside even when heading safely toward the
			// fence. Permit progress toward the nearest boundary, otherwise turn inward.
			Vector2d nearest;
			size_t nearestEdge = 0;
			const double distance = distanceToPolygon(poly, p0, &nearest, &nearestEdge);
			const Vector2d edge = poly[(nearestEdge + 1) % poly.size()] - poly[nearestEdge];
			outward = winding * Vector2d(edge.y(), -edge.x()).normalized();
			reflected = nearest.normalized();
			const bool approaches = direction.dot(reflected) > kEps &&
				(!m_status.bPredictedBreach || distanceToPolygon(poly, p1) < distance - kBoundaryEpsM);
			m_status.bTurnRequired = !approaches;
		}
		else if (m_status.bPredictedBreach)
		{
			// Numerical fallback at an edge: use its inward normal if no crossing was found.
			size_t nearestEdge = 0;
			distanceToPolygon(poly, p0, nullptr, &nearestEdge);
			const Vector2d edge = poly[(nearestEdge + 1) % poly.size()] - poly[nearestEdge];
			outward = winding * Vector2d(edge.y(), -edge.x()).normalized();
			reflected = -outward;
			m_status.bTurnRequired = true;
		}

		if (outward.norm() > kEps)
		{
			m_status.attackDeg = static_cast<float>(signedAngleDeg(heading - vectorToHeadingDeg(outward)));
			m_status.reflectedHdgDeg = static_cast<float>(vectorToHeadingDeg(reflected));
			m_status.turnDeg = static_cast<float>(signedAngleDeg(m_status.reflectedHdgDeg - heading));
		}
	}

}
