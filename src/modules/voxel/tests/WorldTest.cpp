/**
 * @file
 */

#include "AbstractVoxelTest.h"
#include "voxel/World.h"
#include "config.h"
#include <chrono>
#include <string>

namespace voxel {

class WorldTest: public AbstractVoxelTest {
private:
	int _chunkMeshPositionTest = 0;
protected:
	void extract(int expected) {
		World world;
		core::Var::get(cfg::VoxelMeshSize, "16", core::CV_READONLY);
		const io::FilesystemPtr& filesystem = _testApp->filesystem();
		ASSERT_TRUE(world.init(filesystem->load("world.lua"), filesystem->load("biomes.lua")));
		world.setSeed(0);
		world.setPersist(false);
		for (int i = 0; i < expected; ++i) {
			const glm::ivec3 pos { i * 1024, 0, i };
			ASSERT_TRUE(world.scheduleMeshExtraction(pos)) << "Failed to schedule mesh extraction for " << glm::to_string(pos);
		}

		int meshes;
		int extracted;
		int pending;
		world.stats(meshes, extracted, pending);

		extracted = 0;
		auto start = std::chrono::high_resolution_clock::now();
		for (;;) {
			ChunkMeshes meshData(0, 0, 0, 0);
			while (!world.pop(meshData)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
#if USE_GPROF == 0
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = end - start;
				const double millis = elapsed.count();
				ASSERT_LT(millis, 120 * 1000) << "Took too long to got a finished mesh from the queue";
#endif
			}
			world.stats(meshes, extracted, pending);
			if (extracted == expected) {
				break;
			}
			ASSERT_GT(pending, 0) << "Nothing left in the mesh creation phase";
		}
		world.shutdown();
	}

	void chunkMeshPositionTest(
			const World& world,
			int worldX, int worldY, int worldZ,
			int chunkX, int chunkY, int chunkZ,
			int meshX,  int meshY,  int meshZ
			) {
		SCOPED_TRACE("Testcase call: " + std::to_string(++_chunkMeshPositionTest));

		const glm::ivec3 vec(worldX, worldY, worldZ);

		const glm::ivec3& chunkPos = world.chunkPos(vec);
		ASSERT_EQ(glm::ivec3(chunkX, chunkY, chunkZ), chunkPos)
			<< "Chunk position doesn't match the expected for chunk size: " << world.chunkSize()
			<< " at: " << vec.x << ", " << vec.y << ", " << vec.z;

		const glm::ivec3& meshPos = world.meshPos(vec);
		ASSERT_EQ(glm::ivec3(meshX, meshY, meshZ), meshPos)
			<< "Mesh position doesn't match the expected for mesh size: " << world.meshSize()
			<< " at: " << vec.x << ", " << vec.y << ", " << vec.z;
	}
};

TEST_F(WorldTest, testExtractionMultiple) {
	extract(4);
}

TEST_F(WorldTest, testExtractionSingle) {
	extract(1);
}

// e.g. chunksize = 64 and meshsize = 64
// 0 - 63 => chunk 0
// -64 - -1 => chunk -1
// 0 - 63 => meshPos 0
// -64 - -1 => meshPos -64
TEST_F(WorldTest, testChunkAndmeshPos) {
	World world;
	core::Var::get(cfg::VoxelMeshSize, "16", core::CV_READONLY);
	const io::FilesystemPtr& filesystem = _testApp->filesystem();
	ASSERT_TRUE(world.init(filesystem->load("world.lua"), filesystem->load("biomes.lua")));
	const int chunkSize = world.chunkSize();
	//const int halfChunkSize = chunkSize / 2;
	const int meshSize = world.meshSize();
	ASSERT_EQ(0, chunkSize % meshSize) << "mesh size must be a multiple of chunk size";
	const int meshFactor = chunkSize / meshSize;
	ASSERT_GT(meshFactor, 0) << "mesh factor is <= 0, which mean, <= 0 meshes fitting into a chunk - weird";
	//const int chunkSizePlusOneMesh = chunkSize + meshSize;
	const int chunkSizeMinusOneMesh = chunkSize - meshSize;

	chunkMeshPositionTest(world, chunkSize, chunkSize, chunkSize, 1, 1, 1, chunkSize, chunkSize, chunkSize);
	chunkMeshPositionTest(world, chunkSize + 1, chunkSize + 1, chunkSize + 1, 1, 1, 1, chunkSize, chunkSize, chunkSize);
	chunkMeshPositionTest(world, chunkSize - 1, chunkSize - 1, chunkSize - 1, 0, 0, 0, chunkSizeMinusOneMesh, chunkSizeMinusOneMesh, chunkSizeMinusOneMesh);
	chunkMeshPositionTest(world, -chunkSize, -chunkSize, -chunkSize, -1, -1, -1, -chunkSize, -chunkSize, -chunkSize);
	chunkMeshPositionTest(world, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	chunkMeshPositionTest(world, -1, -1, -1, -1, -1, -1, -meshSize, -meshSize, -meshSize);
#if 0
	chunkMeshPositionTest(world, -(chunkSize + 1), -(chunkSize + 1), -(chunkSize + 1), -2, -2, -2, -2 * chunkSize, -2 * chunkSize, -2 * chunkSize);
	chunkMeshPositionTest(world, -halfChunkSize, halfChunkSize, halfChunkSize, -1, 0, 0, -chunkSize, 0, 0);
	chunkMeshPositionTest(world, -halfChunkSize, -halfChunkSize, -halfChunkSize, -1, -1, -1, -chunkSize, 0, -chunkSize);
	chunkMeshPositionTest(world, halfChunkSize, halfChunkSize, halfChunkSize, 0, 0, 0, 0, 0, 0);

	chunkMeshPositionTest(world, 2 * chunkSize + halfChunkSize, 2 * chunkSize + halfChunkSize, 2 * chunkSize + halfChunkSize, 2, 2, 2, 2 * chunkSize, 0, 2 * chunkSize);
	chunkMeshPositionTest(world, halfChunkSize, 0, halfChunkSize, 0, 0, 0, 0, 0, 0);
	chunkMeshPositionTest(world, halfChunkSize, MAX_HEIGHT - 1, halfChunkSize, 0, MAX_HEIGHT / chunkSize, 0, 0, 0, 0);
#endif
}

}
