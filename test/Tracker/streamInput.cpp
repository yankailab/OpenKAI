#include "../../src/Tracker/_TrackerBase.h"
#include "../../src/UI/_Console.h"
#include <cassert>

namespace kai
{
	void _Console::addMsg(const string &, int)
	{
	}

	void _Console::addMsg(const string &, int, int, int)
	{
	}

	InstanceMgr::InstanceMgr() = default;
	InstanceMgr::~InstanceMgr() = default;

	void *InstanceMgr::findDataStream(const string &)
	{
		return nullptr;
	}

	void *InstanceMgr::findModule(const string &)
	{
		return nullptr;
	}
}

namespace
{
	class TestTracker : public kai::_TrackerBase
	{
	public:
		TestTracker(kai::RGBframe &input)
		{
			m_pRGBin = &input;
		}

		bool check(void) override
		{
			return m_pRGBin != nullptr;
		}

		cv::Rect2d target(void) const
		{
			return m_newBB;
		}
	};
}

int main()
{
	kai::RGBframe input;
	TestTracker tracker(input);
	Eigen::Vector4f bounds(0.25f, 0.25f, 0.75f, 0.75f);
	assert(!tracker.startTrack(bounds));

	cv::Mat capture(80, 120, CV_8UC3, cv::Scalar(10, 20, 30));
	input.set(capture, 100);
	const auto held = input.get();
	const cv::Mat original = held->m_mRGB.clone();
	assert(tracker.startTrack(bounds));
	assert(tracker.target() == cv::Rect2d(30, 20, 60, 40));

	// Rendering owns its mutable target; annotations never change a published
	// capture retained by this tracker or another stream consumer.
	cv::Mat annotated = held->m_mRGB.clone();
	tracker.draw(&annotated);
	assert(cv::norm(annotated, original, cv::NORM_INF) > 0);
	assert(cv::norm(held->m_mRGB, original, cv::NORM_INF) == 0);
	assert(input.get() == held);

	// A new publication changes dimensions without mutating an older frame.
	input.set(cv::Mat(40, 60, CV_8UC3, cv::Scalar(40, 50, 60)), 100);
	assert(tracker.startTrack(bounds));
	assert(tracker.target() == cv::Rect2d(15, 10, 30, 20));
	assert(held->m_mRGB.size() == cv::Size(120, 80));
	assert(cv::norm(held->m_mRGB, original, cv::NORM_INF) == 0);

	input.set(cv::Mat(), 101);
	assert(!tracker.startTrack(bounds));
	assert(cv::norm(held->m_mRGB, original, cv::NORM_INF) == 0);
	return 0;
}
