#include "TestMesh.h"

TestMesh::TestMesh(io::FilesystemPtr filesystem, core::EventBusPtr eventBus) :
		Super(filesystem, eventBus) {
}

core::AppState TestMesh::onInit() {
	const core::AppState state = Super::onInit();

	if (!_meshShader.setup()) {
		Log::error("Failed to init mesh shader");
		return core::AppState::Cleanup;
	}

	const std::string mesh = "animal_rabbit.dae";
	if (!_mesh.loadMesh(mesh)) {
		Log::error("Failed to load the mesh %s", mesh.c_str());
		return core::AppState::Cleanup;
	}

	return state;
}

void TestMesh::doRender() {
	video::ScopedShader scoped(_meshShader);
	_meshShader.setView(_camera.viewMatrix());
	_meshShader.setProjection(_camera.projectionMatrix());
	_meshShader.setFogrange(500.0f);
	_meshShader.setViewdistance(500.0f);
	_meshShader.setModel(glm::mat4());
	_meshShader.setTexture(0);

	if (!_mesh.initMesh(_meshShader)) {
		Log::error("Failed to init the mesh");
		return;
	}
	core_assert(_mesh.render() > 0);
}

core::AppState TestMesh::onCleanup() {
	_meshShader.shutdown();
	_mesh.shutdown();
	return Super::onCleanup();
}

int main(int argc, char *argv[]) {
	return core::getApp<TestMesh>()->startMainLoop(argc, argv);
}