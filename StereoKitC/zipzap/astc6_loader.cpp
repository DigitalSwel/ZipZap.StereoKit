#include "astc6_loader.h"
#include "ktx2_astc6.h"
#include "../sk_memory.h"

#include <sk_renderer.h>

namespace sk {
namespace zipzap {

struct astc6_decode_t {
	ktx2_astc6_t view;
	void* output;
	bool decoded;
};

static bool astc6_prepare(void* data, size_t bytes, ktx2_astc6_t* out_view) {
	if (!ktx2_astc6_open(data, bytes, out_view)) {
		log_warn("KTX2 ASTC6 rejected: invalid header, DFD, ranges or Zstd framing.");
		return false;
	}
	if (!tex_format_supports_sampling(tex_format_astc6x6_rgba_srgb, true)) {
		log_warn("KTX2 ASTC6 sRGB unsupported for sampled upload with linear filtering; select portable content before loading.");
		return false;
	}
	skr_vec3i_t size = { out_view->width, out_view->height, 1 };
	if (skr_tex_calc_mip_size(skr_tex_fmt_astc6x6_rgba_srgb, size, 0) != out_view->decoded_bytes) {
		log_err("KTX2 ASTC6 size disagreement with sk_renderer.");
		return false;
	}
	return true;
}

static void astc6_describe(const ktx2_astc6_t& view, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count) {
	*out_format = tex_format_astc6x6_rgba_srgb;
	*out_width = view.width;
	*out_height = view.height;
	*out_array_count = 1;
	*out_mip_count = 1;
}

bool astc6_matches(const void* data, size_t bytes) {
	return ktx2_astc6_matches(data, bytes);
}

bool astc6_info(void* data, size_t bytes, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count) {
	ktx2_astc6_t view = {};
	if (!astc6_prepare(data, bytes, &view)) return false;
	astc6_describe(view, out_format, out_width, out_height, out_array_count, out_mip_count);
	return true;
}

astc6_decode_t* astc6_decode_begin(void* data, size_t bytes, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count, void** out_data) {
	ktx2_astc6_t view = {};
	if (!astc6_prepare(data, bytes, &view)) return nullptr;

	astc6_decode_t* state = sk_malloc_zero_t(astc6_decode_t, 1);
	state->view = view;
	state->output = sk_malloc(view.decoded_bytes);
	*out_data = state->output;
	astc6_describe(view, out_format, out_width, out_height, out_array_count, out_mip_count);
	return state;
}

bool astc6_decode_step(astc6_decode_t* state, int32_t* out_level, bool* out_done) {
	if (state->decoded || !ktx2_astc6_decode(state->view, state->output, state->view.decoded_bytes)) {
		log_warn("KTX2 ASTC6 decode failed: invalid block payload or Zstd checksum.");
		return false;
	}
	state->decoded = true;
	*out_level = 0;
	*out_done = true;
	return true;
}

void astc6_decode_end(astc6_decode_t* state) {
	sk_free(state);
}

}
}
