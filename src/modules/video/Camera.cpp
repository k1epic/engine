/**
 * @file
 */

#include "Camera.h"
#include "core/Common.h"
#include "core/Singleton.h"
#include "io/EventHandler.h"
#include "core/GLM.h"

namespace video {

Camera::Camera(CameraType type, CameraMode mode) :
	_type(type), _mode(mode), _pos(glm::vec3()), _omega(0.0f) {
}

Camera::~Camera() {
}

void Camera::init(const glm::ivec2& position, const glm::ivec2& dimension) {
	if (_position == position && _dimension == dimension) {
		return;
	}
	_position = position;
	_dimension = dimension;
	_aspectRatio = _dimension.x / static_cast<float>(_dimension.y);
	_dirty = DIRTY_ALL;
}

void Camera::move(const glm::vec3& delta) {
	if (glm::all(glm::epsilonEqual(delta, glm::vec3(), 0.0001f))) {
		return;
	}
	_dirty |= DIRTY_POSITON;
	_pos += forward() * -delta.z;
	_pos += right() * delta.x;
	_pos += up() * delta.y;
	if (_rotationType == CameraRotationType::Target) {
		lookAt(_target, glm::up);
		_dirty |= DIRTY_TARGET;
	}
}

void Camera::rotate(const glm::vec3& radians) {
	// TODO: what about rotationtype... should matter here, too, no?
	switch(_type) {
	case CameraType::FirstPerson:
		turn(radians.y);
		pitch(radians.x);
		break;
	case CameraType::Free:
		yaw(radians.y);
		pitch(radians.x);
		roll(radians.z);
		break;
	}
}

inline void Camera::slerp(const glm::quat& quat, float factor) {
	_quat = glm::mix(_quat, quat, factor);
	_dirty |= DIRTY_ORIENTATION;
	core_assert(!glm::any(glm::isnan(_quat)));
	core_assert(!glm::any(glm::isinf(_quat)));
}

void Camera::slerp(const glm::vec3& radians, float factor) {
	const glm::quat quat2(radians);
	_quat = glm::mix(_quat, quat2, factor);
	_dirty |= DIRTY_ORIENTATION;
	core_assert(!glm::any(glm::isnan(_quat)));
	core_assert(!glm::any(glm::isinf(_quat)));
}

bool Camera::lookAt(const glm::vec3& position, const glm::vec3& upDirection) {
	if (glm::all(glm::epsilonEqual(_pos, position, 0.0001f))) {
		return false;
	}

	glm::vec3 targetDir = glm::normalize(position - _pos);
	if (glm::length2(targetDir) == 0) {
		targetDir = glm::forward;
	}

	glm::vec3 upDir(glm::uninitialize);
	if (glm::epsilonEqual(glm::length2(upDirection), 0.0f, glm::epsilon<float>())) {
		upDir = glm::up;
	} else {
		upDir = upDirection;
	}

	if (glm::epsilonEqual(glm::length2(glm::cross(upDir, targetDir)), 0.0f, glm::epsilon<float>())) {
		upDir = glm::cross(targetDir, glm::right);
		if (glm::epsilonEqual(glm::length2(upDir), 0.0f, glm::epsilon<float>())) {
			upDir = glm::cross(targetDir, glm::backward);
		}
	}

	_quat = glm::quat_cast(glm::lookAt(_pos, _pos + targetDir, upDir));
	_dirty |= DIRTY_ORIENTATION;
	core_assert_msg(!glm::any(glm::isnan(_quat)), "upDirection(%f:%f:%f), position(%f:%f:%f), _pos(%f:%f:%f)",
			upDirection.x, upDirection.y, upDirection.z, position.x, position.y, position.z, _pos.x, _pos.y, _pos.z);
	core_assert_msg(!glm::any(glm::isinf(_quat)), "upDirection(%f:%f:%f), position(%f:%f:%f), _pos(%f:%f:%f)",
			upDirection.x, upDirection.y, upDirection.z, position.x, position.y, position.z, _pos.x, _pos.y, _pos.z);
	return true;
}

void Camera::billboard(glm::vec3 *right, glm::vec3 *up) const {
	const glm::mat4& view = viewMatrix();
	*right = glm::vec3(glm::row(view, 0));
	*up = glm::vec3(glm::row(view, 1));
}

void Camera::updateTarget() {
	if (_rotationType != CameraRotationType::Target) {
		return;
	}
	if (!isDirty(DIRTY_ORIENTATION | DIRTY_TARGET)) {
		return;
	}
	const glm::vec3& backward = -forward();
	const glm::vec3& newPosition = _target + backward * _distance;
	if (glm::all(glm::epsilonEqual(_pos, newPosition, 0.0001f))) {
		return;
	}
	_pos = newPosition;
	_dirty |= DIRTY_POSITON;
}

void Camera::update(long deltaFrame) {
	const float dt = deltaFrame / 1000.0f;
	if (dt > 0.0f) {
		rotate(_omega * dt);
	}
	updateTarget();
	updateOrientation();
	updateViewMatrix();
	updateProjectionMatrix();
	updateFrustumPlanes();
	updateFrustumVertices();
	_viewProjectionMatrix = projectionMatrix() * viewMatrix();
	_dirty = 0;
}

void Camera::updateOrientation() {
	if (!isDirty(DIRTY_ORIENTATION)) {
		return;
	}

	_quat = glm::normalize(_quat);
	_orientation = glm::mat4_cast(_quat);
}

void Camera::updateProjectionMatrix() {
	if (!isDirty(DIRTY_PERSPECTIVE)) {
		return;
	}
	switch(_mode) {
	case CameraMode::Orthogonal:
		_projectionMatrix = orthogonalMatrix();
		break;
	case CameraMode::Perspective:
		_projectionMatrix = perspectiveMatrix();
		break;
	}
}

void Camera::updateViewMatrix() {
	if (!isDirty(DIRTY_ORIENTATION | DIRTY_POSITON)) {
		return;
	}
	_viewMatrix = glm::translate(orientation(), -_pos);
}

Ray Camera::mouseRay(const glm::ivec2& screenPos) const {
	return screenRay(glm::vec2(screenPos.x / (float)width(), screenPos.y / (float)height()));
	/*const glm::vec2 newPos = glm::vec2(screenPos - _position) / glm::vec2(dimension());
	return screenRay(newPos);*/
}

Ray Camera::screenRay(const glm::vec2& screenPos) const {
	// project screen position [0.0-1.0] to [-1.0,1.0] and flip y axis
	// to bring them into homogeneous clip coordinates
	const float x = (2.0f * screenPos.x) - 1.0f;
	const float y = 1.0f - (2.0f * screenPos.y);
	const glm::vec4 rayClipSpace(x, y, -1.0f, 1.0f);

	glm::vec4 rayEyeSpace = glm::inverse(projectionMatrix()) * rayClipSpace;
	rayEyeSpace = glm::vec4(glm::vec2(rayEyeSpace), -1.0f, 0.0f);

	const glm::vec3& rayDirection = glm::normalize(glm::vec3(glm::inverse(viewMatrix()) * rayEyeSpace));
	return Ray(position(), rayDirection);
}

glm::vec3 Camera::screenToWorld(const glm::vec3& screenPos) const {
	const Ray& ray = screenRay(glm::vec2(screenPos));
	return ray.origin + ray.direction * screenPos.z;
}

/**
 * http://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf
 */
void Camera::sliceFrustum(float* sliceBuf, int bufSize, int splits, float sliceWeight) const {
	core_assert_always(bufSize >= splits * 2);
	core_assert_always(splits >= 1);
	const float near = nearPlane();
	const float far = farPlane();

	int bufIdx = 0;
	for (int split = 0; split < splits; ++split) {
		const float nearK = float(bufIdx) / splits;
		const float nearLogd = near * glm::pow(far / near, nearK);
		const float nearLind = glm::mix(near, far, nearK);
		const float nearSplitVal = glm::mix(nearLogd, nearLind, sliceWeight);
		sliceBuf[bufIdx++] = nearSplitVal;

		const float farK = float(bufIdx) / splits;
		const float farLogd = near * glm::pow(far / near, farK);
		const float farLind = glm::mix(near, far, farK);
		const float farSplitVal = glm::mix(farLogd, farLind, sliceWeight);
		sliceBuf[bufIdx++] = farSplitVal;
	}
}

void Camera::splitFrustum(float nearPlane, float farPlane, glm::vec3 out[core::FRUSTUM_VERTICES_MAX]) const {
	glm::mat4 proj(glm::uninitialize);
	switch(_mode) {
	case CameraMode::Orthogonal:
		proj = glm::ortho(0.0f, (float)width(), (float)height(), 0.0f, nearPlane, farPlane);
		break;
	case CameraMode::Perspective:
		proj = glm::perspective(glm::radians(_fieldOfView), _aspectRatio, nearPlane, farPlane);
		break;
	}

	const glm::mat4& transform = glm::inverse(proj * viewMatrix());
	frustum().split(transform, out);
}

void Camera::updateFrustumVertices() {
	if (!isDirty(DIRTY_ORIENTATION | DIRTY_POSITON | DIRTY_PERSPECTIVE)) {
		return;
	}

	_frustum.updateVertices(viewMatrix(), projectionMatrix());
}

void Camera::updateFrustumPlanes() {
	if (!isDirty(DIRTY_ORIENTATION | DIRTY_POSITON | DIRTY_PERSPECTIVE)) {
		return;
	}

	_frustum.updatePlanes(viewMatrix(), projectionMatrix());
}

core::AABB<float> Camera::aabb() const {
	core_assert(!isDirty(DIRTY_ORIENTATION | DIRTY_POSITON | DIRTY_PERSPECTIVE));
	return frustum().aabb();
}

static inline float getBoundingSphereRadius(const glm::vec3& center, const std::vector<glm::vec3>& points) {
	float radius = 0.0f;
	for (const glm::vec3& p : points) {
		radius = glm::max(radius, glm::distance(center, p));
	}
	return radius;
}

glm::vec4 Camera::splitFrustumSphereBoundingBox(float near, float far) const {
	const glm::mat4& projection = projectionMatrix();
	const glm::mat4& inverseProjection = glm::inverse(projection);

	const float znearp = glm::project(projection, glm::vec3(0.0f, 0.0f, -near)).z;
	const float zfarp = glm::project(projection, glm::vec3(0.0f, 0.0f, -far)).z;

	std::vector<glm::vec3> points(8);

	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 2; ++y) {
			for (int z = 0; z < 2; ++z) {
				const glm::vec3 v(x ? 1 : -1, y ? 1 : -1, z ? zfarp : znearp);
				const glm::vec3& p = glm::project(inverseProjection, v);
				points.emplace_back(p);
			}
		}
	}

	const glm::vec3& begin = glm::project(inverseProjection, glm::vec3(0.0f, 0.0f, znearp));
	const glm::vec3& end = glm::project(inverseProjection, glm::vec3(0.0f, 0.0f, zfarp));
	float radiusBegin = getBoundingSphereRadius(begin, points);
	float radiusEnd = getBoundingSphereRadius(end, points);

	float rangeBegin = 0.0f;
	float rangeEnd = 1.0f;

	while (rangeEnd - rangeBegin > 1e-3) {
		const float rangeMiddle = (rangeBegin + rangeEnd) / 2.0f;
		const float radiusMiddle = getBoundingSphereRadius(glm::mix(begin, end, rangeMiddle), points);

		if (radiusBegin < radiusEnd) {
			radiusEnd = radiusMiddle;
			rangeEnd = rangeMiddle;
		} else {
			radiusBegin = radiusMiddle;
			rangeBegin = rangeMiddle;
		}
	}

	return glm::vec4(glm::mix(begin, end, rangeBegin), radiusBegin);
}

glm::vec4 Camera::sphereBoundingBox() const {
	const core::AABB<float> boundingBox = aabb();
	const glm::vec3& mins = boundingBox.getLowerCorner();
	const glm::vec3& maxs = boundingBox.getUpperCorner();

	const glm::vec3 sphereCenter(
			mins.x + (maxs.x - mins.x) / 2.0f,
			mins.y + (maxs.x - mins.y) / 2.0f,
			mins.z + (maxs.z - mins.z) / 2.0f);
	const float sphereRadius = std::max({
			(maxs.x - mins.x) / 2.0f,
			(maxs.y - mins.y) / 2.0f,
			(maxs.z - mins.z) / 2.0f});

	return glm::vec4(sphereCenter, sphereRadius);
}

glm::mat4 Camera::orthogonalMatrix() const {
	const float _x = x();
	const float _y = y();
	const float w = _x + width();
	const float h = _y + height();
	core_assert_msg(w > 0.0f, "Invalid dimension given: width must be greater than zero but is %f", w);
	core_assert_msg(h > 0.0f, "Invalid dimension given: height must be greater than zero but is %f", h);
	return glm::ortho(_x, w, h, _y, nearPlane(), farPlane());
}

glm::mat4 Camera::perspectiveMatrix() const {
	return glm::perspective(glm::radians(_fieldOfView), _aspectRatio, nearPlane(), farPlane());
}

void Camera::setNearPlane(float nearPlane) {
	if (glm::epsilonEqual(_nearPlane, nearPlane, 0.00001f)) {
		return;
	}

	_dirty |= DIRTY_PERSPECTIVE;
	if (_mode == CameraMode::Orthogonal) {
		_nearPlane = nearPlane;
	} else {
		_nearPlane = std::max(0.1f, nearPlane);
	}
}

void Camera::setFarPlane(float farPlane) {
	if (glm::epsilonEqual(_farPlane, farPlane, 0.00001f)) {
		return;
	}

	_dirty |= DIRTY_PERSPECTIVE;
	_farPlane = farPlane;
}

}
