#include "WorldGenerator.h"
#include "noise/Noise.h"

namespace voxel {
namespace world {

static float getHeight(const glm::vec2& noisePos2d, const WorldContext& worldCtx) {
	const float landscapeNoise = ::noise::Noise2D(noisePos2d, worldCtx.landscapeNoiseOctaves,
			worldCtx.landscapeNoisePersistence, worldCtx.landscapeNoiseFrequency, worldCtx.landscapeNoiseAmplitude);
	const float noiseNormalized = ::noise::norm(landscapeNoise);
	const float mountainNoise = ::noise::Noise2D(noisePos2d, worldCtx.mountainNoiseOctaves,
			worldCtx.mountainNoisePersistence, worldCtx.mountainNoiseFrequency, worldCtx.mountainNoiseAmplitude);
	const float mountainNoiseNormalized = ::noise::norm(mountainNoise);
	const float mountainMultiplier = mountainNoiseNormalized * (mountainNoiseNormalized + 0.5f);
	const float n = glm::clamp(noiseNormalized * mountainMultiplier, 0.0f, 1.0f);
	return n;
}

int fillVoxels(int x, int lowerY, int z, const WorldContext& worldCtx, Voxel* voxels, BiomeManager& biomManager, long seed, int noiseSeedOffsetX, int noiseSeedOffsetZ, int maxHeight) {
	const glm::vec2 noisePos2d(noiseSeedOffsetX + x, noiseSeedOffsetZ + z);
	const int ni = getHeight(noisePos2d, worldCtx) * maxHeight;
	if (ni < lowerY) {
		return 0;
	}

	const Voxel& water = createColorVoxel(VoxelType::Water, seed);
	const Voxel& dirt = createColorVoxel(VoxelType::Dirt, seed);
	static constexpr Voxel air;

	// TODO: apply city gradient from biome manager
	// TODO: support region y parameters

	voxels[0] = dirt;
	for (int y = ni - 1; y >= lowerY + 1; --y) {
		const glm::vec3 noisePos3d(noisePos2d.x, y, noisePos2d.y);
		const float noiseVal = ::noise::norm(
				::noise::Noise3D(noisePos3d, worldCtx.caveNoiseOctaves, worldCtx.caveNoisePersistence,
						worldCtx.caveNoiseFrequency, worldCtx.caveNoiseAmplitude));
		const float finalDensity = n + noiseVal;
		if (finalDensity > worldCtx.caveDensityThreshold) {
			const bool cave = y < ni - 1;
			const glm::ivec3 pos(x, y, z);
			const Voxel& voxel = biomManager.getVoxel(pos, cave);
			voxels[y] = voxel;
		} else {
			if (y < MAX_WATER_HEIGHT) {
				voxels[y] = water;
			} else {
				voxels[y] = air;
			}
		}
	}
	for (int i = lowerY; i < MAX_WATER_HEIGHT; ++i) {
		if (voxels[i] == air) {
			voxels[i] = water;
		}
	}
	return std::max(ni - lowerY, MAX_WATER_HEIGHT - lowerY);
}

}
}
