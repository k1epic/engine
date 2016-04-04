#pragma once

namespace video {

// https://github.com/mattdesl/lwjgl-basics/wiki/GLSL-Versions
constexpr struct Versions {
	int minor;
	int major;
	int glslVersion;
} GLVersions[] = {
	{2, 0, 110},
	{2, 1, 120},
	{3, 0, 130},
	{3, 1, 140},
	{3, 2, 150},
	{3, 3, 330},
	{4, 0, 400},
	{4, 1, 410},
	{4, 2, 420},
	{4, 3, 430}
};

}
