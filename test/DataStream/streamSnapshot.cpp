#include "../../src/DataStream/PCLframe.h"
#include "../../src/DataStream/LineFrame.h"
#include "../../src/DataStream/PCLmap.h"
#include "../../src/DataStream/IMUstream.h"
#include <atomic>
#include <cassert>
#include <limits>
#include <thread>

using namespace kai;

namespace
{
	void testPointSnapshots(void)
	{
		PCLframe stream;
		const auto initial = stream.get();
		assert(initial && initial->m_revision == 0 && initial->m_vPoints.empty());
		vector<GEOMETRY_POINT> points(4);
		points[0].m_vP = Vector3f(1.0f, 2.0f, 3.0f);
		points[0].m_tStamp = 100;
		const GEOMETRY_POINT *pStorage = points.data();
		stream.set(std::move(points), 100);
		const auto first = stream.get();
		assert(first->m_vPoints.data() == pStorage);
		assert(first->m_revision == 1 && first->m_tStamp == 100);
		stream.set({}, 100);
		assert(stream.get()->m_revision == 2);
		assert(stream.get()->m_vPoints.empty());
		stream.set({}, 1);
		assert(stream.get()->m_revision == 3);
		assert(stream.get()->m_tStamp == 1);
		assert(first->m_vPoints.size() == 4);
		assert(first->m_vPoints[0].m_vP == Vector3f(1.0f, 2.0f, 3.0f));
		assert(initial->m_revision == 0);

		vector<GEOMETRY_POINT> copy;
		stream.set(first->m_vPoints, 25);
		copy = stream.get()->m_vPoints;
		assert(copy.size() == 4 && copy[0].m_tStamp == 100);
		assert(stream.get()->m_revision == 4 && stream.get()->m_tStamp == 25);
	}

	void testLineSnapshots(void)
	{
		LineFrame stream;
		const auto initial = stream.get();
		assert(initial && initial->m_revision == 0 && initial->m_vLines.empty());
		vector<GEOMETRY_LINE> lines(3);
		lines[0].m_vPa = Vector3f(1.0f, 2.0f, 3.0f);
		lines[0].m_vPb = Vector3f(4.0f, 5.0f, 6.0f);
		lines[0].m_tStamp = 50;
		const GEOMETRY_LINE *pStorage = lines.data();
		stream.set(std::move(lines), 50);
		const auto first = stream.get();
		assert(first->m_vLines.data() == pStorage);
		assert(first->m_revision == 1 && first->m_tStamp == 50);
		stream.set({}, 50);
		assert(stream.get()->m_revision == 2 && stream.get()->m_vLines.empty());
		stream.set({}, 1);
		assert(stream.get()->m_revision == 3 && stream.get()->m_tStamp == 1);
		assert(first->m_vLines.size() == 3);
		assert(first->m_vLines[0].m_vPb == Vector3f(4.0f, 5.0f, 6.0f));
		vector<GEOMETRY_LINE> copy;
		stream.set(first->m_vLines, 10);
		copy = stream.get()->m_vLines;
		assert(copy.size() == 3 && copy[0].m_tStamp == 50);
	}

	void testMapSnapshots(void)
	{
		PCLmap stream;
		assert(stream.get()->m_revision == 0);
		auto points = std::make_shared<vector<Vector3f>>();
		points->push_back(Vector3f(1.0f, 2.0f, 3.0f));
		PCLmap::Submap submap;
		submap.m_id = 7;
		submap.m_tStamp = 25;
		submap.m_points = points;
		stream.set({submap}, 1, 25);
		const auto first = stream.get();
		assert(first->m_vSubmaps[0].m_points.get() == points.get());
		submap.m_pose.translation() = Vector3d(5.0, 6.0, 7.0);
		stream.set({submap}, 1, 25);
		const auto moved = stream.get();
		assert(moved->m_revision == first->m_revision + 1);
		assert(moved->m_vSubmaps[0].m_points.get() == first->m_vSubmaps[0].m_points.get());
		assert(first->m_vSubmaps[0].m_pose.translation().isZero());
		assert(moved->m_vSubmaps[0].m_pose.translation() == Vector3d(5.0, 6.0, 7.0));
		stream.set({}, 2, 1);
		assert(stream.get()->m_session == 2);
		assert(stream.get()->m_revision == moved->m_revision + 1);
		assert(stream.get()->m_vSubmaps.empty());
		assert(first->m_vSubmaps[0].m_points->size() == 1);
	}

	void publishFrames(PCLframe *pStream, std::atomic<bool> *pDone)
	{
		for (uint64_t frame = 1; frame <= 2000; ++frame)
		{
			vector<GEOMETRY_POINT> points(16);
			for (GEOMETRY_POINT &point : points)
			{
				point.m_vP = Vector3f::Constant(static_cast<float>(frame));
				point.m_vC = Vector3f::Constant(static_cast<float>(frame + 1));
				point.m_tStamp = frame;
			}
			pStream->set(std::move(points), frame);
		}
		pDone->store(true);
	}

	void testConcurrentSnapshots(void)
	{
		PCLframe stream;
		std::atomic<bool> done{false};
		std::thread writer(publishFrames, &stream, &done);
		uint64_t previousRevision = 0;
		do
		{
			const auto snapshot = stream.get();
			assert(snapshot->m_revision >= previousRevision);
			previousRevision = snapshot->m_revision;
			if (snapshot->m_revision == 0)
			{
				assert(snapshot->m_vPoints.empty());
				continue;
			}
			assert(snapshot->m_revision == snapshot->m_tStamp);
			assert(snapshot->m_vPoints.size() == 16);
			for (const GEOMETRY_POINT &point : snapshot->m_vPoints)
			{
				assert(point.m_tStamp == snapshot->m_tStamp);
				assert(point.m_vP == Vector3f::Constant(static_cast<float>(snapshot->m_tStamp)));
				assert(point.m_vC == Vector3f::Constant(static_cast<float>(snapshot->m_tStamp + 1)));
			}
		}
		while (!done.load());
		writer.join();
		assert(stream.get()->m_revision == 2000);
	}

	void testIMUreaders(void)
	{
		IMUstream stream;
		const auto initial = stream.get();
		assert(initial->m_revision == 0 && initial->m_dqGyro.empty() && initial->m_dqAcc.empty());
		const Vector3f value(1.0f, 2.0f, 3.0f);
		stream.set(IMUstream::Type::Gyro, value, 100);
		stream.set(IMUstream::Type::Acc, value, 107);
		const auto first = stream.get();
		const auto second = stream.get();
		assert(first->m_revision == 2 && first->m_tStamp == 107);
		assert(second->m_dqGyro.front().m_t == 100 && second->m_dqAcc.front().m_t == 107);
		stream.set(IMUstream::Type::Gyro, value, 300);
		stream.set(IMUstream::Type::Acc, value, 305);
		assert(first->m_dqGyro.size() == 1 && first->m_dqAcc.size() == 1);
		assert(initial->m_revision == 0 && initial->m_dqGyro.empty());
		const auto appended = stream.get();
		assert(appended->m_dqGyro.size() == 2 && appended->m_dqAcc.size() == 2);

		// Resetting one capture clock discards only that sensor's old history.
		// Sequence positions keep increasing so consumers can detect new data.
		stream.set(IMUstream::Type::Gyro, value, 10);
		const auto resetGyro = stream.get();
		assert(resetGyro->m_dqGyro.size() == 1 && resetGyro->m_dqAcc.size() == 2);
		assert(resetGyro->m_dqGyro.front().m_sequence > appended->m_revision);
		stream.set(IMUstream::Type::Acc, value, 12);
		const auto reset = stream.get();
		assert(reset->m_dqGyro.front().m_t == 10 && reset->m_dqAcc.front().m_t == 12);
		assert(reset->m_revision == 6);
		stream.set(IMUstream::Type::Gyro, value, 10);
		assert(stream.get()->m_revision == 7);
		assert(reset->m_revision == 6 && reset->m_dqAcc.front().m_t == 12);
	}

	void testIMUlimits(void)
	{
		IMUstream stream;
		const Vector3f value(1.0f, 2.0f, 3.0f);
		for (uint64_t stamp = 1; stamp <= 1100; ++stamp)
		{
			stream.set(IMUstream::Type::Gyro, value, stamp);
			stream.set(IMUstream::Type::Acc, value, stamp);
		}
		Vector3f invalid = value;
		invalid.x() = std::numeric_limits<float>::quiet_NaN();
		stream.set(IMUstream::Type::Gyro, invalid, 1101);
		stream.set(IMUstream::Type::Acc, invalid, 1101);
		stream.set(IMUstream::Type::Gyro, value, 0);
		stream.set(IMUstream::Type::Acc, value, 0);
		const auto frame = stream.get();
		assert(frame->m_revision == 2200 && frame->m_tStamp == 1100);
		assert(frame->m_dqGyro.size() == 1000 && frame->m_dqAcc.size() == 1000);
		for (size_t i = 0; i < 1000; ++i)
		{
			assert(frame->m_dqGyro[i].m_t == 101 + i);
			assert(frame->m_dqAcc[i].m_t == 101 + i);
			assert(frame->m_dqGyro[i].m_sequence == (101 + i) * 2 - 1);
			assert(frame->m_dqAcc[i].m_sequence == (101 + i) * 2);
		}
	}

}

int main(void)
{
	testPointSnapshots();
	testLineSnapshots();
	testMapSnapshots();
	testConcurrentSnapshots();
	testIMUreaders();
	testIMUlimits();
	return 0;
}
