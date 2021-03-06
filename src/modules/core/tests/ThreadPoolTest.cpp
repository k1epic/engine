/**
 * @file
 */

#include "AbstractTest.h"
#include "core/ThreadPool.h"

namespace core {

class ThreadPoolTest: public AbstractTest {
public:
	std::atomic_int _count;
	bool _executed;
	ThreadPoolTest() :
		_count(0), _executed(false) {
	}
};

TEST_F(ThreadPoolTest, testPush) {
	core::ThreadPool pool(1);
	auto future = pool.enqueue([this] () {_executed = true;});
	future.get();
	ASSERT_TRUE(_executed) << "Thread wasn't executed";
}

TEST_F(ThreadPoolTest, testMultiplePush) {
	const int x = 1000;
	{
		core::ThreadPool pool(2);
		for (int i = 0; i < x; ++i) {
			pool.enqueue([this] () {_count++;});
		}
	}
	ASSERT_EQ(x, _count) << "Not all threads were executed";
}

}
