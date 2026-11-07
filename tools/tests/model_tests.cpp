// SPDX-License-Identifier: MIT
#include "model_tests.h"

#include <stereokit.h>
#include <stdio.h>
#include <string.h>

using namespace sk;

static int model_failures = 0;
static int model_checks   = 0;

#define MODEL_CHECK(condition, description) do { \
	model_checks += 1; \
	if (!(condition)) { \
		log_errf("[model_test] FAIL: %s", description); \
		model_failures += 1; \
	} \
	} while (0)

static bool info_equals(model_t model, model_node_id node, const char* key, const char* expected) {
	const char* value = model_node_info_get(model, node, key);
	return value != nullptr && strcmp(value, expected) == 0;
}

static void test_metadata_copy() {
	model_t       source = model_create();
	model_node_id root   = model_node_add(source, "root", matrix_identity);
	model_node_id child  = model_node_add_child(source, root, "child", matrix_identity);
	model_node_id empty  = model_node_add_child(source, root, "empty", matrix_identity);
	model_node_info_set(source, root, "asset.name", "Train");
	model_node_info_set(source, root, "authoring_only", "true");
	model_node_info_set(source, root, "empty", "");
	model_node_info_set(source, root, "utf8", "train-\xc4\x8d");
	model_node_info_set(source, root, "removed", "temporary");
	model_node_info_remove(source, root, "removed");
	model_node_info_set(source, child, "socket", "{\"position\":[0,1,5]}");
	for (int i = 0; i < 32; i++) {
		char key[32];
		snprintf(key, sizeof(key), "slot-%d", i);
		model_node_info_set(source, root, key, "value");
	}

	model_t first  = model_copy(source);
	model_t second = model_copy(source);
	MODEL_CHECK(model_node_count(first) == 3, "node graph retained");
	MODEL_CHECK(model_node_info_count(first, root) == 36, "all metadata entries copied");
	MODEL_CHECK(model_node_info_count(first, empty) == 0, "empty metadata stays empty");
	MODEL_CHECK(info_equals(first, root, "empty", ""), "empty value copied");
	MODEL_CHECK(info_equals(first, root, "utf8", "train-\xc4\x8d"), "UTF-8 value copied");
	MODEL_CHECK(model_node_info_get(first, root, "removed") == nullptr, "removed key not copied");
	for (int i = 0; i < 32; i++) {
		char key[32];
		snprintf(key, sizeof(key), "slot-%d", i);
		MODEL_CHECK(info_equals(first, root, key, "value"), "entry survives dictionary growth");
		MODEL_CHECK(model_node_info_get(first, root, key) != model_node_info_get(source, root, key),
			"copied value has independent storage");
	}

	model_node_info_set(first, root, "authoring_only", "false");
	model_node_info_remove(first, root, "asset.name");
	model_node_info_clear(first, child);
	model_node_info_set(first, root, "copy-only", "yes");
	MODEL_CHECK(info_equals(source, root, "authoring_only", "true"), "copy edit leaves source unchanged");
	MODEL_CHECK(info_equals(second, root, "authoring_only", "true"), "copy edit leaves sibling unchanged");
	MODEL_CHECK(info_equals(source, root, "asset.name", "Train"), "copy removal leaves source key");
	MODEL_CHECK(info_equals(second, child, "socket", "{\"position\":[0,1,5]}"), "copy clear leaves sibling metadata");
	MODEL_CHECK(model_node_info_get(source, root, "copy-only") == nullptr, "new copy key is not shared");

	model_node_info_set(source, root, "asset.name", "Changed source");
	model_node_info_remove(source, root, "empty");
	MODEL_CHECK(info_equals(second, root, "asset.name", "Train"), "source edit leaves copy unchanged");
	MODEL_CHECK(info_equals(second, root, "empty", ""), "source removal leaves copy key");
	model_release(source);
	model_release(first);
	sk_step(nullptr);
	MODEL_CHECK(info_equals(second, root, "asset.name", "Train"), "copy survives source and sibling destruction");

	model_t third = model_copy(second);
	model_release(second);
	sk_step(nullptr);
	MODEL_CHECK(info_equals(third, child, "socket", "{\"position\":[0,1,5]}"), "copy of copy owns its metadata");
	model_release(third);
}

static void test_copy_destroyed_first() {
	for (int i = 0; i < 64; i++) {
		model_t       source = model_create();
		model_node_id node   = model_node_add(source, "node", matrix_identity);
		model_node_info_set(source, node, "owner", "source");
		model_t copy = model_copy(source);
		model_release(copy);
		sk_step(nullptr);
		MODEL_CHECK(info_equals(source, node, "owner", "source"), "source survives copy destruction");
		model_release(source);
	}
	model_t empty = model_create();
	model_t copy  = model_copy(empty);
	MODEL_CHECK(model_node_count(copy) == 0, "empty model copies safely");
	model_release(empty);
	model_release(copy);
}

int model_tests_run() {
	model_failures = 0;
	model_checks   = 0;
	sk_settings_t settings = {};
	settings.app_name     = "StereoKitC Model Tests";
	settings.mode         = app_mode_offscreen;
	settings.standby_mode = standby_mode_none;
	if (!sk_init(settings)) {
		log_err("[model_test] sk_init failed");
		return 1;
	}

	test_metadata_copy();
	test_copy_destroyed_first();
	sk_step(nullptr);
	sk_shutdown();
	printf("[model_test] %d checks; %d failures\n", model_checks, model_failures);
	return model_failures;
}
