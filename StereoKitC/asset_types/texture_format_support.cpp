#include "../stereokit.h"
#include "../_stereokit.h"

#include <sk_renderer.h>
#if defined(SKR_VK)
#include <volk.h>
#endif

namespace sk {

bool32_t tex_format_supports_sampling(tex_format_ format, bool32_t linear_filter) {
	if (!sk_is_initialized() || format == tex_format_none) return false;
#if defined(SKR_VK)
	VkPhysicalDevice device = static_cast<VkPhysicalDevice>(backend_vulkan_get_physical_device());
	VkFormat native = static_cast<VkFormat>(skr_tex_fmt_to_native(static_cast<skr_tex_fmt_>(format)));
	if (!device || native == VK_FORMAT_UNDEFINED) return false;

	// Match an uploaded, readable 2D texture, not just a compression-family bit.
	skr_tex_flags_ flags = static_cast<skr_tex_flags_>(skr_tex_flags_readable | skr_tex_flags_dynamic);
	if (!skr_tex_fmt_is_supported(static_cast<skr_tex_fmt_>(format), flags, 1)) return false;
	VkFormatProperties properties = {};
	vkGetPhysicalDeviceFormatProperties(device, native, &properties);
	VkFormatFeatureFlags required = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
	if (linear_filter) required |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
	return (properties.optimalTilingFeatures & required) == required;
#else
	// No concrete filtering query is available in the pinned WebGPU adapter.
	// Unknown support must not select native-only content.
	return false;
#endif
}

}
