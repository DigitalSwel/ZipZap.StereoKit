#include "ktx2_astc6.h"

#include <limits.h>
#include <string.h>

// Same private decoder as the general KTX path. Framing is validated before
// this decoder can allocate a window or consume concatenated frames.
extern "C" {
size_t   ZSTD_decompress(void* dst, size_t dst_capacity, const void* src, size_t src_size);
unsigned ZSTD_isError(size_t code);
}

namespace sk {
namespace {

const uint8_t identifier[] = { 0xab, 'K', 'T', 'X', ' ', '2', '0', 0xbb, 13, 10, 26, 10 };
const uint64_t maximum_level_bytes = 256ULL * 1024 * 1024;
const uint32_t astc6_srgb_vk_format = 166;

uint32_t read32(const uint8_t* p) {
	return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}
uint64_t read64(const uint8_t* p) {
	return uint64_t(read32(p)) | uint64_t(read32(p + 4)) << 32;
}
bool zero(const uint8_t* p, size_t bytes) {
	for (size_t i = 0; i < bytes; i++) if (p[i] != 0) return false;
	return true;
}
bool range(size_t total, uint64_t offset, uint64_t bytes) {
	return bytes <= total && offset <= total - bytes;
}

bool valid_zstd(const uint8_t* p, size_t bytes, size_t expected) {
	if (bytes < 5 || read32(p) != 0xfd2fb528) return false;
	uint8_t descriptor = p[4];
	if (descriptor & 0x1b) return false; // dictionary ID and reserved bits
	bool single = (descriptor & 0x20) != 0;
	size_t cursor = 5;
	if (!single) {
		if (cursor >= bytes) return false;
		uint8_t window = p[cursor++];
		uint64_t base = 1ULL << (10 + (window >> 3));
		if (base + base / 8 * (window & 7) > maximum_level_bytes) return false;
	}
	int flag = descriptor >> 6;
	size_t size_bytes = flag == 0 ? (single ? 1 : 0) : size_t(1) << flag;
	if (size_bytes > bytes - cursor) return false;
	if (size_bytes != 0) {
		uint64_t size = 0;
		for (size_t i = 0; i < size_bytes; i++) size |= uint64_t(p[cursor + i]) << (8 * i);
		if (size_bytes == 2) size += 256;
		if (size != expected) return false;
		cursor += size_bytes;
	}
	bool last = false;
	uint32_t blocks = 0;
	while (!last) {
		if (++blocks > 65536 || bytes - cursor < 3) return false;
		uint32_t header = uint32_t(p[cursor]) | uint32_t(p[cursor+1]) << 8 | uint32_t(p[cursor+2]) << 16;
		cursor += 3;
		last = (header & 1) != 0;
		uint32_t type = (header >> 1) & 3, size = header >> 3;
		if (type == 3 || size > 128 * 1024) return false;
		size_t stored = type == 1 ? 1 : size;
		if (stored > bytes - cursor) return false;
		cursor += stored;
	}
	size_t checksum_bytes = (descriptor & 4) ? 4 : 0;
	return bytes - cursor == checksum_bytes;
}

}

bool ktx2_astc6_matches(const void* data, size_t bytes) {
	const uint8_t* p = static_cast<const uint8_t*>(data);
	return p && bytes >= 16 && memcmp(p, identifier, sizeof(identifier)) == 0 && read32(p + 12) == astc6_srgb_vk_format;
}

bool ktx2_astc6_open(const void* data, size_t bytes, ktx2_astc6_t* out_view) {
	if (!out_view || bytes < 148 || !ktx2_astc6_matches(data, bytes)) return false;
	const uint8_t* p = static_cast<const uint8_t*>(data);
	uint32_t width = read32(p + 20), height = read32(p + 24), scheme = read32(p + 44);
	if (read32(p + 16) != 1 || !width || !height || width > INT32_MAX || height > INT32_MAX
		|| read32(p + 28) != 0 || read32(p + 32) != 0 || read32(p + 36) != 1 || read32(p + 40) != 1
		|| (scheme != 0 && scheme != 2) || read64(p + 64) != 0 || read64(p + 72) != 0) return false;

	// Native textures may be NPOT: ceil independently, not a game SBS alignment rule.
	uint64_t decoded = ((uint64_t(width) + 5) / 6) * ((uint64_t(height) + 5) / 6) * 16;
	uint64_t dfd = read32(p + 48), dfd_bytes = read32(p + 52), kvd = read32(p + 56), kvd_bytes = read32(p + 60);
	uint64_t level = read64(p + 80), stored = read64(p + 88);
	if (!decoded || decoded > maximum_level_bytes || !stored || stored > maximum_level_bytes
		|| dfd < 104 || dfd_bytes != 44 || !range(bytes, dfd, dfd_bytes)
		|| level < 104 || !range(bytes, level, stored) || stored != bytes - level
		|| read64(p + 96) != decoded || (scheme == 0 && (level % 16 != 0 || stored != decoded))
		|| dfd + dfd_bytes > level || kvd_bytes > 1024 * 1024
		|| (kvd_bytes && (kvd < 104 || !range(bytes, kvd, kvd_bytes) || kvd + kvd_bytes > level
			|| !(kvd + kvd_bytes <= dfd || dfd + dfd_bytes <= kvd)))) return false;

	const uint8_t* d = p + size_t(dfd);
	if (read32(d) != 44 || read32(d + 4) != 0 || d[8] != 2 || d[9] != 0 || d[10] != 40 || d[11] != 0
		|| d[12] != 162 || d[13] != 1 || d[14] != 2 || d[15] != 0
		|| d[16] != 5 || d[17] != 5 || d[18] != 0 || d[19] != 0
		|| d[20] != 16 || !zero(d + 21, 9) || d[30] != 127 || d[31] != 0
		|| read32(d + 32) != 0 || read32(d + 36) != 0 || read32(d + 40) != UINT32_MAX) return false;

	const uint8_t* blocks = p + size_t(level);
	if (scheme == 2 && !valid_zstd(blocks, size_t(stored), size_t(decoded))) return false;
	*out_view = { blocks, size_t(stored), size_t(decoded), int32_t(width), int32_t(height), scheme == 2 };
	return true;
}

bool ktx2_astc6_decode(const ktx2_astc6_t& view, void* output, size_t output_bytes) {
	if (!output || !view.level || output_bytes != view.decoded_bytes) return false;
	if (!view.zstd) {
		if (view.stored_bytes != output_bytes) return false;
		memcpy(output, view.level, output_bytes);
		return true;
	}
	size_t result = ZSTD_decompress(output, output_bytes, view.level, view.stored_bytes);
	return !ZSTD_isError(result) && result == output_bytes;
}

}
