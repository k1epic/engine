/**
 * @file
 */

#pragma once

#include "GLFunc.h"
#include <glm/vec2.hpp>

namespace video {

enum class DepthBufferMode {
	// stores -1..1 window-space depth values
	RGBA,
	// stores 0..1 window-space depth values
	DEPTH,
	DEPTH_CMP
};

class DepthBuffer {
public:
	DepthBuffer();
	~DepthBuffer();

	bool init(const glm::ivec2& dimension, DepthBufferMode mode = DepthBufferMode::RGBA, int textureCount = 1);
	void shutdown();

	void bind(bool read = false, int textureIndex = 0);
	void unbind();
	uint8_t *read();

	inline const glm::ivec2& dimension() const {
		return _dimension;
	}

	inline GLuint getTexture(int index = 0) const {
		core_assert(index >= 0 && index < (int)SDL_arraysize(_depthTexture));
		return _depthTexture[index];
	}

	inline bool depthAttachment() const {
		return _mode == DepthBufferMode::DEPTH || _mode == DepthBufferMode::DEPTH_CMP;
	}

private:
	GLint _viewport[4] = {0, 0, 0, 0};
	GLint _oldFramebuffer = -1;
	GLuint _fbo = 0u;
	GLuint _rbo = 0u;
	GLuint _depthTexture[4] = {0u, 0u, 0u, 0u};
	glm::ivec2 _dimension;
	DepthBufferMode _mode = DepthBufferMode::RGBA;
};

}
