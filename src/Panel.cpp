#ifdef __linux__
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkwayland.h>
#include <dlfcn.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <Shlwapi.h>
#include <io.h>

#define access _access_s
#endif

#ifdef __APPLE__
#include <libgen.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <unistd.h>
#include "CocoaSurface.h"
#endif

#ifdef __linux__
#include <limits.h>
#include <libgen.h>
#include <unistd.h>

#if defined(__sun)
#define PROC_SELF_EXE "/proc/self/path/a.out"
#else
#define PROC_SELF_EXE "/proc/self/exe"
#endif
#endif

#include "Panel.h"

static const VkPipelineVertexInputStateCreateInfo vertexInfo = {
	VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	nullptr,
	NULL,
	1,
	(VkVertexInputBindingDescription[1]){
		{
			0, // binding
			8,//sizeof(Vertex), // stride
			VK_VERTEX_INPUT_RATE_VERTEX // input rate
		}
	},
	1,
	(VkVertexInputAttributeDescription[1]){
		{
			0, 0, VK_FORMAT_R32G32_SFLOAT, 0//offsetof(Vertex, position)
		}
	}
};

static const float32_t triangleVertices[6] = {
	1.0f, 1.0f,
	-1.0f, 1.0f,
	0.0f, -1.0f
};
static uint32_t triangleIndices[3] = {0, 1, 2};

BEGIN_EVENT_TABLE(Panel, wxPanel)
//EVT_MOUSE_EVENTS(VulkanCanvas::OnMouse)
//EVT_KEY_DOWN(VulkanCanvas::OnKeyDown)
//EVT_KEY_UP(VulkanCanvas::OnKeyUp)
//EVT_KILL_FOCUS(VulkanCanvas::OnKillFocus)
END_EVENT_TABLE()

Panel::Panel(wxWindow *pParent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
: wxPanel(pParent, id, pos, size, style, name){
	
	Bind(wxEVT_PAINT, &Panel::OnPaint, this);
	Bind(wxEVT_SIZE, &Panel::OnResize, this);
	Bind(wxEVT_ERASE_BACKGROUND, &Panel::OnEraseBG, this);
	
	vkDevices = ConstructVkDevices();
	
	vkInterface = ConstructVkInterface();
	
	vkInterface->FillVertexBuffer(0, (void *)triangleVertices, sizeof(triangleVertices));
	vkInterface->FillIndexBuffer(0, triangleIndices, 3);
}

void Panel::OnPaint(wxPaintEvent &event){
	DrawFrame();
}
void Panel::OnResize(wxSizeEvent &event){
	vkInterface->FramebufferResizeCallback();
	DrawFrame();
}
void Panel::OnEraseBG(wxEraseEvent &event){
	
}
void Panel::OnPaintException(const std::string &message){
	wxMessageBox(message, "Vulkan Error");
	wxTheApp->ExitMainLoop();
}

void Panel::DrawFrame(){
	if(vkInterface->BeginFrame()){
		
		vkInterface->BeginFinalRenderPass({{0.0f, 0.0f, 0.0f, 1.0f}});
		
		vkInterface->GP(0).Bind();
		vkInterface->CmdBindVertexBuffer(0, 0);
		vkInterface->CmdBindIndexBuffer(0, VK_INDEX_TYPE_UINT32);
		vkInterface->CmdDrawIndexed(3);
		
		vkInterface->EndFinalRenderPassAndFrame();
	}
}

std::shared_ptr<EVK::Devices> Panel::ConstructVkDevices(){
	return std::make_shared<EVK::Devices>("Volume_Renderer2", GetRequiredExtensions(),
										  [this](VkInstance instance) -> VkSurfaceKHR {
	  gui_initHandleContextFromWxWidgetsWindow(g_window_info.canvas_main, this);
#ifdef __linux__
	  if (g_window_info.canvas_main.backend == WindowHandleInfo::Backend::WAYLAND)
	  {
		  m_subsurface = std::make_unique<wxWlSubsurface>(this);
		  canvasMain.wayland_surface = m_subsurface->getSurface();
	  }
#endif
	  return CreateFramebufferSurface(instance, g_window_info.canvas_main);
  },
											 [this]() -> VkExtent2D {
	  int width, height;
	  GetSize(&width, &height);
	  return (VkExtent2D){
		  .width = uint32_t(width),
		  .height = uint32_t(height)
	  };
  });
}
std::shared_ptr<EVK::Interface> Panel::ConstructVkInterface(){
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	
	//If depthClampEnable is set to VK_TRUE, then fragments that are beyond the near and far planes are clamped to them as opposed to discarding them. This is useful in some special cases like shadow maps.
	
	//Using this requires enabling a GPU feature.
	
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	
	//The polygonMode determines how fragments are generated for geometry. The following modes are available:
	
	//VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
	//VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
	//VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
	
	//Using any mode other than fill requires enabling a GPU feature.
	
	rasterizer.lineWidth = 1.0f;
	//The lineWidth member is straightforward, it describes the thickness of lines in terms of number of fragments. The maximum line width that is supported depends on the hardware and any line thicker than 1.0f requires you to enable the wideLines GPU feature.
	
	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	//The cullMode variable determines the type of face culling to use. You can disable culling, cull the front faces, cull the back faces or both. The frontFace variable specifies the vertex order for faces to be considered front-facing and can be clockwise or counterclockwise.
	
	//rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	
	
	// ----- Multisampling behaviour -----
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	//#ifdef MSAA
	//multisampling.rasterizationSamples = device.GetMSAASamples();
	//#else
	//multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	//#endif
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional
	// This is how to achieve anti-aliasing.
	// Enabling it requires enabling a GPU feature
	
	
	// ----- Colour blending behaviour -----
	VkPipelineColorBlendAttachmentState colourBlendAttachment{};
	colourBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colourBlendAttachment.blendEnable = VK_TRUE;
	colourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	
	VkPipelineColorBlendStateCreateInfo colourBlending{};
	colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colourBlending.logicOpEnable = VK_FALSE;
	colourBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
//	colourBlending.attachmentCount = 1;
	colourBlending.pAttachments = &colourBlendAttachment;
	colourBlending.blendConstants[0] = 0.0f; // Optional
	colourBlending.blendConstants[1] = 0.0f; // Optional
	colourBlending.blendConstants[2] = 0.0f; // Optional
	colourBlending.blendConstants[3] = 0.0f; // Optional
	
	
	// ----- Depth stencil behaviour -----
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
//	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional
	
	// ----- Dynamic state -----
	VkPipelineDynamicStateCreateInfo dynamicState{};
	VkDynamicState dynamicStates[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;
	
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.pName = "main",
			.pSpecializationInfo = nullptr
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pName = "main",
			.pSpecializationInfo = nullptr
		}
	};
	
	std::vector<VkPipelineShaderStageCreateInfo> mainShaderStages = shaderStages;
	mainShaderStages[0].module = vkDevices->CreateShaderModule("main.vert.spv");
	mainShaderStages[1].module = vkDevices->CreateShaderModule("main.frag.spv");
	colourBlending.attachmentCount = 1;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	dynamicState.dynamicStateCount = 2;
#ifdef MSAA
	multisampling.rasterizationSamples = devices.GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	rasterizer.cullMode = VK_CULL_MODE_NONE;//VK_CULL_MODE_BACK_BIT;
	rasterizer.depthBiasEnable = VK_FALSE;
	EVK::GraphicsPipelineBlueprint pbMain = {
		.pipelineBlueprint.descriptorSetBlueprints = {},
		.pipelineBlueprint.pushConstantRanges = {},
		.shaderStageCIs = mainShaderStages,
		.vertexInputStateCI = vertexInfo,
//		.vertexInputStateCI = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 0, nullptr, 0, nullptr},
		.rasterisationStateCI = rasterizer,
		.multisampleStateCI = multisampling,
		.colourBlendStateCI = colourBlending,
		.depthStencilStateCI = depthStencil,
		.dynamicStateCI = dynamicState,
		.bufferedRenderPassIndex = {},
		.layeredBufferedRenderPassIndex = {},
		.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	};
	
	EVK::InterfaceBlueprint nvi {*vkDevices};
	
	nvi.graphicsPipelinesN = 1;
	nvi.computePipelinesN = 0;
	nvi.uniformBufferObjectsN = 0;
	nvi.storageBufferObjectsN = 0;
	nvi.samplerBlueprints = {};
	nvi.layeredBufferedRenderPassesN = 0;
	nvi.bufferedRenderPassesN = 0;
	nvi.vertexBuffersN = 1;
	nvi.indexBuffersN = 1;
	nvi.imagesN = 0;
	
	std::shared_ptr<EVK::Interface> ret = std::make_shared<EVK::Interface>(nvi);
	
	ret->BuildGraphicsPipeline(0, pbMain);
	
	return ret;
}

std::vector<const char*> Panel::GetRequiredExtensions() {
#ifdef _WIN32
	std::vector<const char*> extensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
#elif defined(__APPLE__)
	std::vector<const char*> extensions = { "VK_KHR_surface", "VK_EXT_metal_surface", "VK_KHR_portability_enumeration"};
#else
	std::vector<const char*> extensions = { "VK_KHR_surface", "VK_KHR_wayland_surface", "VK_KHR_xlib_surface" };
#endif
	return extensions;
}
VkSurfaceKHR Panel::CreateFramebufferSurface(VkInstance instance, struct WindowHandleInfo& windowInfo){
#ifdef _WIN32
	return CreateWinSurface(instance, windowInfo.hwnd);
#elif defined(__linux__)
	if (windowInfo.backend == WindowHandleInfo::Backend::WAYLAND) {
		return CreateWaylandSurface(instance, windowInfo.wayland_display, windowInfo.wayland_surface);
	}
	else {
		return CreateXlibSurface(instance, windowInfo.xlib_display, windowInfo.xlib_window);
	}

#elif defined(__APPLE__)
	return CreateCocoaSurface(instance, windowInfo.handle);
#endif
}

#ifdef _WIN32
VkSurfaceKHR Panel::CreateWinSurface(VkInstance instance, HWND hwindow){
	VkWin32SurfaceCreateInfoKHR sci{};
	sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	sci.hwnd = hwindow;
	sci.hinstance = GetModuleHandle(nullptr);

	VkSurfaceKHR result;
	VkResult err;
	if ((err = vkCreateWin32SurfaceKHR(instance, &sci, nullptr, &result)) != VK_SUCCESS)
	{
		throw VulkanException(err, "Cannot create a Win32 Vulkan surface");
	}

	return result;
}
#endif

#ifdef __linux__
VkSurfaceKHR Panel::CreateWaylandSurface(VkInstance instance, wl_display* wl_display, wl_surface* wl_surface){
	VkWaylandSurfaceCreateInfoKHR sci{};
	sci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	sci.flags = 0;
	sci.display = wl_display;
	sci.surface = wl_surface;

	VkSurfaceKHR result;
	VkResult err;
	if ((err = vkCreateWaylandSurfaceKHR(instance, &sci, nullptr, &result)) != VK_SUCCESS)
	{
		throw VulkanException(err, "Cannot create a Wayland Vulkan surface");
	}

	return result;
}

VkSurfaceKHR Panel::CreateXlibSurface(VkInstance instance, Display* dpy, Window window){
	VkXlibSurfaceCreateInfoKHR sci{};
	sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	sci.flags = 0;
	sci.dpy = dpy;
	sci.window = window;

	VkSurfaceKHR result;
	VkResult err;
	if ((err = vkCreateXlibSurfaceKHR(instance, &sci, nullptr, &result)) != VK_SUCCESS)
	{
		throw VulkanException(err, "Cannot create a X11 Vulkan surface");
	}

	return result;
}
#endif

void Panel::gui_initHandleContextFromWxWidgetsWindow(WindowHandleInfo& handleInfoOut, class wxWindow* wxw){
#ifdef _WIN32
	handleInfoOut.hwnd = wxw->GetHWND();
#elif defined(__linux__)
	/* dynamically retrieve GTK imports so we dont have to include and link the whole lib */
	void (*dyn_gtk_widget_realize)(GtkWidget * widget);
	dyn_gtk_widget_realize = (void(*)(GtkWidget * widget))dlsym(RTLD_NEXT, "gtk_widget_realize");

	GdkWindow* (*dyn_gtk_widget_get_window)(GtkWidget * widget);
	dyn_gtk_widget_get_window = (GdkWindow * (*)(GtkWidget * widget))dlsym(RTLD_NEXT, "gtk_widget_get_window");

	GdkDisplay* (*dyn_gdk_window_get_display)(GdkWindow * widget);
	dyn_gdk_window_get_display = (GdkDisplay * (*)(GdkWindow * window))dlsym(RTLD_NEXT, "gdk_window_get_display");

	bool (*dyn_GDK_IS_WAYLAND_DISPLAY)(GdkDisplay * display);
	dyn_GDK_IS_WAYLAND_DISPLAY = (bool(*)(GdkDisplay * display))dlsym(RTLD_NEXT, "GDK_IS_WAYLAND_DISPLAY");

	bool (*dyn_GDK_IS_X11_DISPLAY)(GdkDisplay * display);
	dyn_GDK_IS_X11_DISPLAY = (bool(*)(GdkDisplay * display))dlsym(RTLD_NEXT, "GDK_IS_X11_DISPLAY");

	wl_display* (*dyn_gdk_wayland_display_get_wl_display)(GdkDisplay * display);
	dyn_gdk_wayland_display_get_wl_display = (wl_display * (*)(GdkDisplay * display))dlsym(RTLD_NEXT, "gdk_wayland_display_get_wl_display");

	wl_surface* (*dyn_gdk_wayland_window_get_wl_surface)(GdkWindow * window);
	dyn_gdk_wayland_window_get_wl_surface = (wl_surface * (*)(GdkWindow * window))dlsym(RTLD_NEXT, "gdk_wayland_window_get_wl_surface");

	Display* (*dyn_gdk_x11_display_get_xdisplay)(GdkDisplay * display);
	dyn_gdk_x11_display_get_xdisplay = (Display * (*)(GdkDisplay * display))dlsym(RTLD_NEXT, "gdk_x11_display_get_xdisplay");

	Window(*dyn_gdk_x11_window_get_xid)(GdkWindow * window);
	dyn_gdk_x11_window_get_xid = (Window(*)(GdkWindow * window))dlsym(RTLD_NEXT, "gdk_x11_window_get_xid");

	//gdk_keyval_name = (const char* (*)(unsigned int))dlsym(RTLD_NEXT, "gdk_keyval_name");

	if (!dyn_gtk_widget_realize || !dyn_gtk_widget_get_window ||
		!dyn_gdk_window_get_display || !dyn_gdk_x11_display_get_xdisplay ||
		!dyn_gdk_x11_window_get_xid || !gdk_keyval_name || !dyn_gdk_wayland_display_get_wl_display || !dyn_gdk_wayland_window_get_wl_surface)
	{
		throw VulkanException(VkResult::VK_SUCCESS, "Unable to load GDK symbols");
		return;
	}

	/* end of imports */

	// get window
	GtkWidget* gtkWidget = (GtkWidget*)wxw->GetHandle(); // returns GtkWidget
	dyn_gtk_widget_realize(gtkWidget);
	GdkWindow* gdkWindow = dyn_gtk_widget_get_window(gtkWidget);
	GdkDisplay* gdkDisplay = dyn_gdk_window_get_display(gdkWindow);
	if (GDK_IS_X11_WINDOW(gdkWindow))
	{
		handleInfoOut.backend = WindowHandleInfo::Backend::X11;
		handleInfoOut.xlib_window = gdk_x11_window_get_xid(gdkWindow);
		handleInfoOut.xlib_display = gdk_x11_display_get_xdisplay(gdkDisplay);
		if (!handleInfoOut.xlib_display)
		{
			throw VulkanException(VkResult::VK_SUCCESS, "Unable to get xlib display");
		}
	}
	else if (GDK_IS_WAYLAND_WINDOW(gdkWindow))
	{
		handleInfoOut.backend = WindowHandleInfo::Backend::WAYLAND;
		handleInfoOut.wayland_surface = gdk_wayland_window_get_wl_surface(gdkWindow);
		handleInfoOut.wayland_display = gdk_wayland_display_get_wl_display(gdkDisplay);
		if (!handleInfoOut.wayland_display)
		{
			throw VulkanException(VkResult::VK_SUCCESS, "Unable to get Wayland display");
		}
	}
	else
	{
		throw VulkanException(VkResult::VK_SUCCESS, "Unsupported GTK backend");

	}

#else
	handleInfoOut.handle = wxw->GetHandle();
#endif
}
