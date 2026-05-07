#include "voxelio/test/random.hpp"
#include "voxelio/test/test.hpp"

#include "voxelio/color.hpp"
#include "voxelio/palette.hpp"

#include <random>
#include <set>
#include <vector>

namespace voxelio::test {
namespace {

[[maybe_unused]] constexpr voxelio::Color32 BLACK = {uint8_t{0}, 0, 0};
[[maybe_unused]] constexpr voxelio::Color32 WHITE = {uint8_t{255u}, 255u, 255u};
[[maybe_unused]] constexpr voxelio::Color32 RED = {uint8_t{255u}, 0, 0};
[[maybe_unused]] constexpr voxelio::Color32 GREEN = {uint8_t{0}, 255u, 0};
[[maybe_unused]] constexpr voxelio::Color32 BLUE = {uint8_t{0}, 0, 255u};

void assertReductionMappingInvariants(const voxelio::Palette32 &palette, const size_t desiredSize)
{
    VXIO_ASSERT_GT(palette.size(), desiredSize);

    size_t actualSize;
    std::unique_ptr<uint32_t[]> reduction = palette.reduce(desiredSize, actualSize);
    VXIO_ASSERT_EQ(actualSize, desiredSize);

    std::vector<uint32_t> originalRepresentatives(palette.size());
    std::set<uint32_t> uniqueRepresentatives;
    for (size_t i = 0; i < palette.size(); ++i) {
        const uint32_t representative = reduction[i];
        VXIO_ASSERT_LT(representative, palette.size());
        originalRepresentatives[i] = representative;
        uniqueRepresentatives.insert(representative);
    }
    VXIO_ASSERT_EQ(uniqueRepresentatives.size(), actualSize);

    voxelio::Palette32 reduced = palette.createReducedPaletteAndStoreMapping(reduction.get());
    VXIO_ASSERT_EQ(reduced.size(), actualSize);

    for (size_t i = 0; i < palette.size(); ++i) {
        VXIO_ASSERT_LT(reduction[i], reduced.size());
        VXIO_ASSERT_EQ(reduced.colorOf(reduction[i]), palette.colorOf(originalRepresentatives[i]));
    }
}

voxelio::Palette32 makeRandomPalette(const size_t colorCount, const uint32_t seed)
{
    voxelio::Palette32 palette;
    default_rng rng{seed};
    std::uniform_int_distribution<uint32_t> distr;
    for (size_t i = 0; i < colorCount; ++i) {
        palette.insert(distr(rng));
    }
    return palette;
}

VXIO_TEST(palette, reduce_2to1)
{
    voxelio::Palette32 palette;
    palette.insert(BLACK);
    palette.insert(WHITE);

    size_t actualSize;
    std::unique_ptr<uint32_t[]> reduction = palette.reduce(1, actualSize);
    voxelio::Palette32 reduced = palette.createReducedPaletteAndStoreMapping(reduction.get());

    VXIO_ASSERT_EQ(actualSize, 1u);
    VXIO_ASSERT_EQ(reduced.size(), 1u);
}

VXIO_TEST(palette, reduce_4to2)
{
    voxelio::Palette32 palette;
    palette.insert(BLACK);
    palette.insert(WHITE);
    palette.insert(RED);
    palette.insert(BLUE);

    size_t actualSize;
    std::unique_ptr<uint32_t[]> reduction = palette.reduce(2, actualSize);
    voxelio::Palette32 reduced = palette.createReducedPaletteAndStoreMapping(reduction.get());

    VXIO_ASSERT_EQ(actualSize, 2u);
    VXIO_ASSERT_EQ(reduced.size(), 2u);
}

VXIO_TEST(palette, reduce_manyRandomTo256)
{
    constexpr size_t desiredSize = 256;
    constexpr size_t colorCount = size_t{1} << (9 + 4 * voxelio::build::RELEASE);
    const voxelio::Palette32 palette = makeRandomPalette(colorCount, 12345);

    size_t actualSize;
    std::unique_ptr<uint32_t[]> reduction = palette.reduce(desiredSize, actualSize);
    voxelio::Palette32 reduced = palette.createReducedPaletteAndStoreMapping(reduction.get());

    VXIO_ASSERT_EQ(actualSize, desiredSize);
    VXIO_ASSERT_EQ(actualSize, reduced.size());
}

VXIO_TEST(palette, reduce_manyBlockTo256)
{
    constexpr size_t desiredSize = 256;
    constexpr size_t colorCount = size_t{1} << (12 + 4 * voxelio::build::RELEASE);

    voxelio::Palette32 palette;

    for (uint32_t i = 0; i < colorCount; ++i) {
        palette.insert(i);
    }

    size_t actualSize;
    std::unique_ptr<uint32_t[]> reduction = palette.reduce(desiredSize, actualSize);
    voxelio::Palette32 reduced = palette.createReducedPaletteAndStoreMapping(reduction.get());

    VXIO_ASSERT_EQ(actualSize, desiredSize);
    VXIO_ASSERT_EQ(actualSize, reduced.size());
}

VXIO_TEST(palette, reduce_manyRandomTo256_mappingInvariants)
{
    constexpr size_t desiredSize = 256;
    constexpr size_t colorCount = size_t{1} << (9 + 4 * voxelio::build::RELEASE);
    const voxelio::Palette32 palette = makeRandomPalette(colorCount, 12345);

    assertReductionMappingInvariants(palette, desiredSize);
}

VXIO_TEST(palette, reduce_manyRandomTo256_mappingInvariants_multipleSeeds)
{
    constexpr size_t desiredSize = 256;
    constexpr size_t colorCount = size_t{1} << (9 + 4 * voxelio::build::RELEASE);
    constexpr uint32_t seeds[] = {1u, 2u, 3u, 5u, 8u, 13u};

    for (uint32_t seed : seeds) {
        const voxelio::Palette32 palette = makeRandomPalette(colorCount, seed);
        assertReductionMappingInvariants(palette, desiredSize);
    }
}

VXIO_TEST(palette, reduce_manyBlockTo256_mappingInvariants)
{
    constexpr size_t desiredSize = 256;
    constexpr size_t colorCount = size_t{1} << (12 + 4 * voxelio::build::RELEASE);

    voxelio::Palette32 palette;
    for (uint32_t i = 0; i < colorCount; ++i) {
        palette.insert(i);
    }

    assertReductionMappingInvariants(palette, desiredSize);
}

}  // namespace
}  // namespace voxelio::test
