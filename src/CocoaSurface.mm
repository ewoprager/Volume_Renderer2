#include "CocoaSurface.h"
#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

#include <iostream>

@interface MetalView : NSView
@end

@implementation MetalView

-(BOOL) wantsUpdateLayer { return YES; }

+(Class) layerClass { return [CAMetalLayer class]; }

-(CALayer*) makeBackingLayer { return [self.class.layerClass layer]; }

@end

VkSurfaceKHR CreateCocoaSurface(VkInstance instance, void *handle) {
	NSView *view = (NSView *)handle;

	MetalView *childView = [[MetalView alloc] initWithFrame:view.bounds];
	childView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
	childView.wantsLayer = YES;

	[view addSubview:childView];

	VkMetalSurfaceCreateInfoEXT surface;
	surface.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
	surface.pNext = NULL;
	surface.flags = 0;
	surface.pLayer = (CAMetalLayer *)childView.layer;

	VkSurfaceKHR result;
	VkResult err;
	if((err = vkCreateMetalSurfaceEXT(instance, &surface, nullptr, &result)) != VK_SUCCESS){
		printf("Cannot create a Metal Vulkan surface: %d", (sint32)err);
		throw std::runtime_error("Cannot create a Metal Vulkan surface:");
	}

	return result;
}
