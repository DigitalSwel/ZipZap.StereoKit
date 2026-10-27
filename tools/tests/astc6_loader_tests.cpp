#include "astc6_loader_tests.h"
#include <stereokit.h>
#include <asset_types/texture_compression.h>
#include <sk_memory.h>

#include <stdio.h>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>

using namespace sk;

static int failures;
#define CHECK(value) do { if (!(value)) { log_errf("[astc6_loader] FAIL line %d: %s", __LINE__, #value); failures++; } } while (0)

static std::vector<uint8_t> fixture(const char* name) {
	std::ifstream stream(std::string(SK_ASTC6_FIXTURES) + "/" + name, std::ios::binary);
	return std::vector<uint8_t>(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

static void check_native(std::vector<uint8_t>& bytes, bool supported) {
	tex_type_ type = tex_type_image_nomips | tex_type_dynamic;
	tex_format_ format = tex_format_none;
	int32_t width = 0, height = 0, images = 0, mips = 0;
	CHECK(ktx2_info(bytes.data(), bytes.size(), &type, &format, &width, &height, &images, &mips) == supported);
	CHECK(type == (tex_type_image_nomips | tex_type_dynamic));
	void* output = nullptr;
	CHECK(ktx2_decode(bytes.data(), bytes.size(), &type, &format, &width, &height, &images, &mips, &output) == supported);
	if (supported) {
		CHECK(format == tex_format_astc6x6_rgba_srgb && width == 60 && height == 24 && images == 1 && mips == 1);
		CHECK(output != nullptr);
	} else CHECK(output == nullptr);
	sk_free(output);

	output = nullptr;
	ktx2_slice_t* slice = ktx2_decode_begin(bytes.data(), bytes.size(), &type, &format, &width, &height, &images, &mips, &output);
	CHECK((slice != nullptr) == supported);
	if (slice) {
		int32_t level = -1;
		bool done = false;
		CHECK(ktx2_decode_step(slice, &level, &done));
		CHECK(level == 0 && done);
		CHECK(!ktx2_decode_step(slice, &level, &done));
	}
	ktx2_decode_end(slice);
	sk_free(output);

	// Cancellation releases only adapter state; the upstream owner still frees output.
	output = nullptr;
	slice = ktx2_decode_begin(bytes.data(), bytes.size(), &type, &format, &width, &height, &images, &mips, &output);
	ktx2_decode_end(slice);
	sk_free(output);

	tex_t texture = tex_create_mem(bytes.data(), bytes.size(), tex_data_srgb, 0);
	CHECK(texture != nullptr);
	assets_block_for_priority(0);
	CHECK((tex_asset_state(texture) == asset_state_loaded) == supported);
	if (supported) {
		CHECK(tex_get_format(texture) == tex_format_astc6x6_rgba_srgb);
		CHECK(tex_get_width(texture) == 60 && tex_get_height(texture) == 24 && tex_get_mips(texture) == 1);
	}
	tex_release(texture);
}

static void check_invalid_native(std::vector<uint8_t> bytes) {
	// Retain the identifier and ASTC6 format, but invalidate the recognized header.
	// It must fail rather than falling through to a different decoder.
	bytes[16] = 0;
	tex_type_ type = tex_type_image_nomips;
	tex_format_ format = tex_format_none;
	int32_t width = 0, height = 0, images = 0, mips = 0;
	CHECK(!ktx2_info(bytes.data(), bytes.size(), &type, &format, &width, &height, &images, &mips));
	void* output = nullptr;
	CHECK(!ktx2_decode(bytes.data(), bytes.size(), &type, &format, &width, &height, &images, &mips, &output));
	CHECK(output == nullptr);
	tex_t texture = tex_create_mem(bytes.data(), bytes.size(), tex_data_srgb, 0);
	assets_block_for_priority(0);
	CHECK(tex_asset_state(texture) < 0);
	tex_release(texture);
}

int astc6_loader_tests_run() {
	failures = 0;
#if defined(SK_ZIPZAP_EXTENSIONS)
	CHECK(!tex_format_supports_sampling(tex_format_astc6x6_rgba_srgb, true));
#endif
	sk_settings_t settings = {};
	settings.app_name = "ASTC6 loader tests";
	settings.mode = app_mode_offscreen;
	settings.standby_mode = standby_mode_none;
	if (!sk_init(settings)) return failures + 1;
	bool supported = false;
#if defined(SK_ZIPZAP_EXTENSIONS)
	supported = tex_format_supports_sampling(tex_format_astc6x6_rgba_srgb, true) != 0;
	CHECK(tex_format_supports_sampling(tex_format_rgba32_srgb, true));
	CHECK(!tex_format_supports_sampling(tex_format_none, true));
#endif
	log_infof("[astc6_loader] ASTC6 sRGB sampled/linear support=%d", supported);
	for (const char* name : { "astc6-none.ktx2", "astc6-zstd9.ktx2" }) {
		auto bytes = fixture(name);
		CHECK(bytes.size() >= 148);
		if (bytes.size() < 148) continue;
		check_native(bytes, supported);
		check_invalid_native(bytes);
	}
	for (const char* name : { "uastc-none.ktx2", "uastc-zstd9.ktx2" }) {
		auto bytes = fixture(name);
		CHECK(!bytes.empty());
		tex_t texture = tex_create_mem(bytes.data(), bytes.size(), tex_data_srgb, 0);
		assets_block_for_priority(0);
		CHECK(tex_asset_state(texture) == asset_state_loaded);
		CHECK(tex_get_width(texture) == 32 && tex_get_height(texture) == 16 && tex_get_mips(texture) == 1);
		log_infof("[astc6_loader] portable %s native format=%d", name, tex_get_format(texture));
		tex_release(texture);
	}
	sk_shutdown();
#if defined(SK_ZIPZAP_EXTENSIONS)
	CHECK(!tex_format_supports_sampling(tex_format_astc6x6_rgba_srgb, true));
#endif
	printf("ASTC6 loader: %d failures\n", failures);
	return failures;
}
