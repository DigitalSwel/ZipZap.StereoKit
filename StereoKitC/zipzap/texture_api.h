#pragma once

#include "../stereokit.h"

#ifdef __cplusplus
namespace sk {
#endif

// ZipZap extension, available in SK_ZIPZAP_EXTENSIONS builds. Query on the SK
// thread after initialization; false when concrete sampling cannot be proved.
SK_API bool32_t tex_format_supports_sampling(tex_format_ format, bool32_t linear_filter sk_default(true));

#ifdef __cplusplus
}
#endif
