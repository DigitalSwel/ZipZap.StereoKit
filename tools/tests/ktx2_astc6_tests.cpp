#include "../../StereoKitC/zipzap/ktx2_astc6.h"

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iterator>
#include <vector>

using namespace sk;

static int failures = 0, checks = 0;
#define CHECK(value) do { checks++; if (!(value)) { printf("FAIL line %d: %s\n", __LINE__, #value); failures++; } } while (0)

static std::vector<uint8_t> fixture(const char* name) {
	std::ifstream stream(std::string(SK_ASTC6_FIXTURES) + "/" + name, std::ios::binary);
	return std::vector<uint8_t>(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}
static uint64_t read64(const std::vector<uint8_t>& bytes, size_t at) {
	uint64_t value = 0;
	for (size_t i = 0; i < 8; i++) value |= uint64_t(bytes[at+i]) << (i*8);
	return value;
}
static void write64(std::vector<uint8_t>& bytes, size_t at, uint64_t value) {
	for (size_t i = 0; i < 8; i++) bytes[at+i] = uint8_t(value >> (i*8));
}
static void write32(std::vector<uint8_t>& bytes, size_t at, uint32_t value) {
	for (size_t i = 0; i < 4; i++) bytes[at+i] = uint8_t(value >> (i*8));
}
static bool open(const std::vector<uint8_t>& bytes) {
	ktx2_astc6_t view = {};
	return ktx2_astc6_open(bytes.data(), bytes.size(), &view);
}
static std::vector<uint8_t> replace_level(const std::vector<uint8_t>& original, const std::vector<uint8_t>& level) {
	size_t offset = size_t(read64(original, 80));
	std::vector<uint8_t> result(original.begin(), original.begin() + offset);
	result.insert(result.end(), level.begin(), level.end());
	write64(result, 88, level.size());
	return result;
}

int main() {
	auto plain = fixture("astc6-none.ktx2"), compressed = fixture("astc6-zstd9.ktx2");
	CHECK(plain.size() == 912 && compressed.size() == 678);
	if (failures) return failures;
	ktx2_astc6_t a = {}, b = {};
	CHECK(ktx2_astc6_open(plain.data(), plain.size(), &a));
	CHECK(ktx2_astc6_open(compressed.data(), compressed.size(), &b));
	CHECK(a.width == 60 && a.height == 24 && a.decoded_bytes == 640 && !a.zstd);
	CHECK(b.width == a.width && b.height == a.height && b.decoded_bytes == a.decoded_bytes && b.zstd);
	std::vector<uint8_t> decoded(640), raw(640);
	CHECK(ktx2_astc6_decode(a, raw.data(), raw.size()));
	CHECK(ktx2_astc6_decode(b, decoded.data(), decoded.size()));
	CHECK(decoded == raw);
	CHECK(!ktx2_astc6_decode(b, decoded.data(), decoded.size() - 1));
	CHECK(!ktx2_astc6_matches(nullptr, 100));
	CHECK(!ktx2_astc6_open(nullptr, 100, &a));

	for (auto original : { plain, compressed }) {
		for (size_t n = 0; n < original.size(); n++) {
			ktx2_astc6_t view = {};
			CHECK(!ktx2_astc6_open(original.data(), n, &view));
		}
		for (size_t offset : { 12u, 16u, 20u, 24u, 28u, 32u, 36u, 40u, 44u, 48u, 52u, 60u, 64u, 72u }) {
			auto bad = original;
			write32(bad, offset, UINT32_MAX);
			CHECK(!open(bad));
		}
		for (size_t offset : { 80u, 88u, 96u }) {
			auto bad = original;
			write64(bad, offset, UINT64_MAX);
			CHECK(!open(bad));
		}
		size_t dfd = original[48] | size_t(original[49]) << 8;
		for (size_t offset = 0; offset < 44; offset++) {
			auto bad = original;
			bad[dfd + offset] ^= 0x10;
			CHECK(!open(bad));
		}
		auto zero_plane = original;
		zero_plane[dfd + 20] = 0;
		CHECK(!open(zero_plane));
		auto overlap = original;
		write32(overlap, 56, uint32_t(dfd));
		write32(overlap, 60, 44);
		CHECK(!open(overlap));
		auto huge = original;
		write32(huge, 20, 2147483647);
		write32(huge, 24, 2147483647);
		CHECK(!open(huge));
	}

	// Native NPOT textures need ceil block dimensions, not divisibility by six.
	auto npot = plain;
	write32(npot, 20, 59);
	write32(npot, 24, 23);
	CHECK(open(npot));
	for (auto original : { plain, compressed }) {
		size_t offset = size_t(read64(original, 80));
		original.insert(original.begin() + offset, 8, 0);
		write64(original, 80, offset + 8);
		CHECK(open(original) == (original[44] == 2));
	}

	size_t offset = size_t(read64(compressed, 80));
	std::vector<uint8_t> level(compressed.begin() + offset, compressed.end());
	for (uint8_t flag : { uint8_t(1), uint8_t(8), uint8_t(16) }) {
		auto bad = level;
		bad[4] |= flag;
		CHECK(!open(replace_level(compressed, bad)));
	}
	auto trailing = level;
	trailing.push_back(0);
	CHECK(!open(replace_level(compressed, trailing)));
	auto concatenated = level;
	concatenated.insert(concatenated.end(), level.begin(), level.end());
	CHECK(!open(replace_level(compressed, concatenated)));
	auto truncated = level;
	truncated.pop_back();
	CHECK(!open(replace_level(compressed, truncated)));
	CHECK(!open(replace_level(compressed, { 0x28, 0xb5, 0x2f, 0xfd, 0, 0xff, 1, 0, 0 })));
	auto checksum = level;
	checksum[4] |= 4;
	checksum.insert(checksum.end(), 4, 0);
	auto checksum_ktx = replace_level(compressed, checksum);
	ktx2_astc6_t checked = {};
	CHECK(ktx2_astc6_open(checksum_ktx.data(), checksum_ktx.size(), &checked));
	CHECK(!ktx2_astc6_decode(checked, decoded.data(), decoded.size()));
	printf("SKKtx2Tests: %d checks, %d failures; ASTC None/Zstd exact 640-byte equality\n", checks, failures);
	return failures ? 1 : 0;
}
