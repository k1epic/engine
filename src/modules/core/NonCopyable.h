/**
 * @file
 */

#pragma once

namespace core {

class NonCopyable {
public:
	NonCopyable() {
	}
private:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
};

}
