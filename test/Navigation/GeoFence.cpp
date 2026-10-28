#include "../../src/Navigation/_GeoFence.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <thread>

namespace
{
	using namespace kai;
	constexpr double degreesPerMeter = 180.0 / (3.14159265358979323846 * 6378137.0);

	Vector2d geo(double east, double north)
	{
		return Vector2d(north * degreesPerMeter, east * degreesPerMeter);
	}

	vector<vector<double>> polygon(const vector<Vector2d> &points)
	{
		vector<vector<double>> result;
		for (const auto &p : points)
		{
			const auto g = geo(p.x(), p.y());
			result.push_back({g.x(), g.y()});
		}
		return result;
	}

	class Fence : public _GeoFence
	{
	public:
		void prediction(float distance) { m_estD = distance; }
	};

	void near(float actual, float expected)
	{
		assert(std::abs(actual - expected) < 0.001f);
	}

	void testBounceAndClosedRings()
	{
		Fence fence;
		fence.prediction(2);
		auto square = polygon({{-10, -10}, {10, -10}, {10, 10}, {-10, 10}, {-10, -10}});
		// Web map rings may repeat the first vertex and contain consecutive duplicates.
		square.insert(square.begin() + 1, square.front());
		for (int winding = 0; winding < 2; ++winding)
		{
			fence.setPolygon(square);
			fence.setPosHdg(geo(9, 0), 45);
			auto status = fence.getStatus();
			assert(status.bValid && !status.bCurrentBreach && status.bPredictedBreach && status.bTurnRequired);
			assert(fence.bBreach());
			near(status.attackDeg, -45);
			near(status.reflectedHdgDeg, 315);
			near(status.turnDeg, -90);

			fence.setPosHdg(geo(9, 0), 135);
			status = fence.getStatus();
			near(status.attackDeg, 45);
			near(status.reflectedHdgDeg, 225);
			near(status.turnDeg, 90);

			fence.setPosHdg(geo(9, 9), 45);
			status = fence.getStatus();
			assert(status.bTurnRequired);
			near(status.reflectedHdgDeg, 225); // reflect both components at a corner

			fence.setPosHdg(geo(10, 0), 90);
			status = fence.getStatus();
			assert(!status.bCurrentBreach && status.bTurnRequired);
			near(status.reflectedHdgDeg, 270); // outward from exactly on the boundary

			fence.setPosHdg(geo(10, 0), 270);
			assert(!fence.getStatus().bTurnRequired && !fence.bBreach());
			fence.setPosHdg(geo(0, 0), 0);
			assert(!fence.bBreach());
			near(fence.getStatus().turnDeg, 0);
			std::reverse(square.begin(), square.end());
		}
	}

	void testOutsideRecovery()
	{
		Fence fence;
		fence.setPolygon(polygon({{-10, -10}, {10, -10}, {10, 10}, {-10, 10}}));
		fence.prediction(1);
		fence.setPosHdg(geo(12, 0), 90);
		auto status = fence.getStatus();
		assert(status.bCurrentBreach && status.bPredictedBreach && status.bTurnRequired);
		near(status.reflectedHdgDeg, 270);
		fence.setPosHdg(geo(12, 0), 270);
		status = fence.getStatus();
		assert(status.bCurrentBreach && status.bPredictedBreach && !status.bTurnRequired);
		assert(fence.bBreach()); // telemetry still reports outside while recovery can advance

		fence.setPosHdg(geo(10.5, 0), 270);
		status = fence.getStatus();
		assert(status.bCurrentBreach && !status.bPredictedBreach && !status.bTurnRequired);
		fence.prediction(30);
		fence.setPosHdg(geo(12, 0), 270);
		status = fence.getStatus();
		assert(status.bCurrentBreach && status.bPredictedBreach && status.bTurnRequired);
		near(status.reflectedHdgDeg, 90); // do not pass through the far boundary
	}

	void testConcaveCrossing()
	{
		Fence fence;
		// U shape: both endpoints are inside opposite arms, but the path crosses the gap.
		fence.setPolygon(polygon({{0, 0}, {6, 0}, {6, 6}, {4, 6}, {4, 2}, {2, 2}, {2, 6}, {0, 6}}));
		fence.prediction(4);
		fence.setPosHdg(geo(1, 4), 90);
		const auto status = fence.getStatus();
		assert(status.bValid && !status.bCurrentBreach && status.bPredictedBreach && status.bTurnRequired);
		near(status.reflectedHdgDeg, 270);
	}

	void testInvalidClearsState()
	{
		Fence fence;
		const auto square = polygon({{-10, -10}, {10, -10}, {10, 10}, {-10, 10}});
		fence.setPolygon(square);
		fence.setPosHdg(geo(10, 0), 90);
		assert(fence.getStatus().bTurnRequired);
		const vector<vector<vector<double>>> invalid = {
			{}, {{0, 0}, {1}, {2, 2}}, {{0, 0}, {0, 0}, {0, 0}},
			{{0, 0}, {0, 1}, {0, 2}}, {{0, 0}, {0, 1}, {100, 1}},
			{{0, 0}, {0, 1}, {std::numeric_limits<double>::quiet_NaN(), 1}}
		};
		for (const auto &points : invalid)
		{
			fence.setPolygon(points);
			const auto status = fence.getStatus();
			assert(!status.bValid && !status.bTurnRequired && !fence.bBreach());
			near(status.turnDeg, 0);
		}
		fence.setPolygon(square);
		fence.setPosHdg(geo(0, 0), std::numeric_limits<float>::infinity());
		assert(!fence.getStatus().bValid);
		fence.setPosHdg(Vector2d(std::numeric_limits<double>::quiet_NaN(), 0), 0);
		assert(!fence.getStatus().bValid);
		fence.prediction(-1);
		fence.setPosHdg(geo(0, 0), 0);
		assert(!fence.getStatus().bValid);
	}

	void testConcurrentUpdates()
	{
		Fence fence;
		const auto square = polygon({{-10, -10}, {10, -10}, {10, 10}, {-10, 10}});
		fence.setPolygon(square);
		std::thread positions([&]() {
			for (unsigned i = 0; i < 1000; ++i)
				fence.setPosHdg(geo(9.5, 0), i % 360);
		});
		std::thread polygons([&]() {
			for (unsigned i = 0; i < 1000; ++i)
				fence.setPolygon(i % 2 ? square : vector<vector<double>>());
		});
		for (unsigned i = 0; i < 1000; ++i)
		{
			const auto status = fence.getStatus();
			assert(std::isfinite(status.reflectedHdgDeg) && std::isfinite(status.turnDeg));
			assert(!status.bTurnRequired || status.bValid);
			if (!status.bValid)
				assert(!status.bCurrentBreach && !status.bPredictedBreach);
		}
		positions.join();
		polygons.join();
	}
}

int main()
{
	testBounceAndClosedRings();
	testOutsideRecovery();
	testConcaveCrossing();
	testInvalidClearsState();
	testConcurrentUpdates();
	std::cout << "GeoFence regressions passed\n";
}
