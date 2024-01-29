#pragma once

#ifdef __APPLE__
#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan.h>

VkSurfaceKHR CreateCocoaSurface(VkInstance instance, void *handle);

#endif
