#pragma once

#include "../stereokit.h"

namespace sk {
namespace zipzap {

// Only recognized native ASTC6 files enter this adapter. Failure is terminal:
// malformed or unsupported native content must not fall through to Basis.
bool astc6_matches(const void* data, size_t bytes);
bool astc6_info(void* data, size_t bytes, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count);

// The adapter owns the handle, not the output allocation. Source bytes must
// outlive the handle; the upstream loader owns and frees *out_data as usual.
struct astc6_decode_t;
astc6_decode_t* astc6_decode_begin(void* data, size_t bytes, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count, void** out_data);
bool astc6_decode_step(astc6_decode_t* state, int32_t* out_level, bool* out_done);
void astc6_decode_end(astc6_decode_t* state);

}
}
