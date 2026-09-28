#include "../../src/DataStream/RGBframe.h"
#include "../../src/DataStream/RGBDframe.h"
#include <atomic>
#include <cassert>
#include <thread>

using namespace kai;

namespace
{
	void testRGBownership(void)
	{
		RGBframe stream;
		const auto initial = stream.get();
		assert(initial && initial->m_revision == 0 && initial->m_mRGB.empty());
		Mat source(4, 6, CV_8UC3, Scalar(10, 20, 30));
		const Mat crop = source(Rect(1, 1, 3, 2));
		stream.set(crop, 100);
		const auto first = stream.get();
		assert(first->m_revision == 1 && first->m_tStamp == 100);
		assert(first->m_mRGB.rows == 2 && first->m_mRGB.cols == 3);
		assert(first->m_mRGB.type() == CV_8UC3);
		assert(first->m_mRGB.data != crop.data);
		assert(first->m_mRGB.at<Vec3b>(0, 0) == Vec3b(10, 20, 30));
		source.setTo(Scalar(40, 50, 60));
		assert(first->m_mRGB.at<Vec3b>(0, 0) == Vec3b(10, 20, 30));
		stream.set(source, 100);
		assert(stream.get()->m_revision == 2 && stream.get()->m_tStamp == 100);
		assert(stream.get()->m_mRGB.at<Vec3b>(0, 0) == Vec3b(40, 50, 60));
		assert(first->m_mRGB.at<Vec3b>(0, 0) == Vec3b(10, 20, 30));
		stream.set(Mat(), 1);
		assert(stream.get()->m_revision == 3 && stream.get()->m_tStamp == 1);
		assert(stream.get()->m_mRGB.empty());
		assert(!first->m_mRGB.empty() && initial->m_mRGB.empty());

		vector<uint8_t> sdkBuffer(8, 7);
		const Mat borrowed(2, 4, CV_8UC1, sdkBuffer.data());
		stream.set(borrowed, 2);
		const auto retained = stream.get();
		sdkBuffer.assign(8, 99);
		assert(retained->m_mRGB.at<uint8_t>(1, 3) == 7);
		assert(retained->m_mRGB.data != sdkBuffer.data());
	}

	void testRGBDownership(void)
	{
		RGBDframe stream;
		const auto initial = stream.get();
		assert(initial && initial->m_revision == 0);
		assert(initial->m_mRGB.empty() && initial->m_mD.empty());
		Mat color(2, 3, CV_8UC3, Scalar(1, 2, 3));
		Mat depth(2, 3, CV_32FC1, Scalar(2.5f));
		stream.set(color, depth, 50);
		const auto first = stream.get();
		assert(first->m_mRGB.data != color.data && first->m_mD.data != depth.data);
		assert(first->m_revision == 1 && first->m_tStamp == 50);
		color.setTo(Scalar(4, 5, 6));
		depth.setTo(7.5f);
		assert(first->m_mRGB.at<Vec3b>(0, 0) == Vec3b(1, 2, 3));
		assert(first->m_mD.at<float>(0, 0) == 2.5f);
		stream.set(color, depth, 50);
		assert(stream.get()->m_revision == 2 && stream.get()->m_tStamp == 50);
		assert(stream.get()->m_mD.at<float>(0, 0) == 7.5f);
		assert(first->m_mD.at<float>(0, 0) == 2.5f);
		stream.set(Mat(), Mat(), 1);
		const auto cleared = stream.get();
		assert(cleared->m_revision == 3 && cleared->m_tStamp == 1);
		assert(cleared->m_mRGB.empty() && cleared->m_mD.empty());
		assert(first->m_mRGB.size() == first->m_mD.size());
	}

	void publishPairs(RGBDframe *pStream, std::atomic<bool> *pDone)
	{
		Mat color(3, 5, CV_32FC1);
		Mat depth(3, 5, CV_32FC1);
		for (uint64_t frame = 1; frame <= 1000; ++frame)
		{
			color.setTo(static_cast<float>(frame));
			depth.setTo(static_cast<float>(frame + 1000));
			pStream->set(color, depth, frame);
		}
		pDone->store(true);
	}

	void testPairedCoherence(void)
	{
		RGBDframe stream;
		std::atomic<bool> done{false};
		std::thread writer(publishPairs, &stream, &done);
		uint64_t revision = 0;
		do
		{
			const auto frame = stream.get();
			assert(frame->m_revision >= revision);
			revision = frame->m_revision;
			if (revision == 0)
			{
				assert(frame->m_mRGB.empty() && frame->m_mD.empty());
				continue;
			}
			assert(frame->m_revision == frame->m_tStamp);
			assert(frame->m_mRGB.size() == frame->m_mD.size());
			for (int row = 0; row < frame->m_mRGB.rows; ++row)
			{
				for (int col = 0; col < frame->m_mRGB.cols; ++col)
				{
					assert(frame->m_mRGB.at<float>(row, col) == static_cast<float>(frame->m_tStamp));
					assert(frame->m_mD.at<float>(row, col) == static_cast<float>(frame->m_tStamp + 1000));
				}
			}
		}
		while (!done.load());
		writer.join();
		assert(stream.get()->m_revision == 1000);
	}
}

int main(void)
{
	testRGBownership();
	testRGBDownership();
	testPairedCoherence();
	return 0;
}
