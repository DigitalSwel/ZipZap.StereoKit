#pragma once

#include <stddef.h>
#include <stdint.h>

namespace sk {

// Borrowed view: source bytes must outlive decode. No GPU or allocation in open.
struct ktx2_astc6_t {
	const uint8_t* level;
	size_t stored_bytes;
	size_t decoded_bytes;
	int32_t width;
	int32_t height;
	bool zstd;
};

bool ktx2_astc6_matches(const void* data, size_t bytes);
bool ktx2_astc6_open(const void* data, size_t bytes, ktx2_astc6_t* out_view);
bool ktx2_astc6_decode(const ktx2_astc6_t& view, void* output, size_t output_bytes);

}
