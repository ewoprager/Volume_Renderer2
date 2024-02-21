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

#include "Frame.h"
#include "VolumeRendering.h"

#include "MainPanel.h"

struct Vertex {
	vec<2> position;
	vec<2> texCoord;
	
	constexpr static const VkPipelineVertexInputStateCreateInfo attributeInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		NULL,
		1,
		(VkVertexInputBindingDescription[1]){
			{
				0, // binding
				16, // stride
				VK_VERTEX_INPUT_RATE_VERTEX // input rate
			}
		},
		2,
		(VkVertexInputAttributeDescription[2]){
			{
				0, 0, VK_FORMAT_R32G32_SFLOAT, 0
			},
			{
				1, 0, VK_FORMAT_R32G32_SFLOAT, 8
			}
		}
	};
};
static constexpr uint32_t verticesN = 4;
static const Vertex triangleVertices[verticesN] = {
	{{ 1.0f,-1.0f}, { 1.0f, 0.0f}},
	{{ 1.0f, 1.0f}, { 1.0f, 1.0f}},
	{{-1.0f, 1.0f}, { 0.0f, 1.0f}},
	{{-1.0f,-1.0f}, { 0.0f, 0.0f}}
};
static constexpr uint32_t indicesN = 6;
static uint32_t triangleIndices[indicesN] = {0, 1, 2, 0, 2, 3};


//struct TestVertex {
//	vec<2> position;
//	
//	constexpr static const VkPipelineVertexInputStateCreateInfo attributeInfo = {
//		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
//		nullptr,
//		NULL,
//		1,
//		(VkVertexInputBindingDescription[1]){
//			{
//				0, // binding
//				8, // stride
//				VK_VERTEX_INPUT_RATE_VERTEX // input rate
//			}
//		},
//		1,
//		(VkVertexInputAttributeDescription[1]){
//			{
//				0, 0, VK_FORMAT_R32G32_SFLOAT, 0
//			}
//		}
//	};
//};
//static constexpr uint32_t testVerticesN = 3;
//static const TestVertex testTriangleVertices[verticesN] = {
//	{{ 0.5f, 0.5f}},
//	{{ 0.0f,-0.5f}},
//	{{-0.5f, 0.5f}}
//};


BEGIN_EVENT_TABLE(MainPanel, wxPanel)
EVT_MOUSE_EVENTS(MainPanel::OnMouse)
//EVT_KEY_DOWN(VulkanCanvas::OnKeyDown)
//EVT_KEY_UP(VulkanCanvas::OnKeyUp)
//EVT_KILL_FOCUS(VulkanCanvas::OnKillFocus)
END_EVENT_TABLE()

MainPanel::MainPanel(Frame *_frameParent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
: wxPanel(_frameParent, id, pos, size, style, name), frameParent(_frameParent) {
	
	Bind(wxEVT_PAINT, &MainPanel::OnPaint, this);
	Bind(wxEVT_SIZE, &MainPanel::OnResize, this);
	Bind(wxEVT_ERASE_BACKGROUND, &MainPanel::OnEraseBG, this);
	Bind(wxEVT_MOUSEWHEEL, &MainPanel::OnMouseWheel, this);
	
	ConstructVkDevices();
	
	vkInterface = ConstructVkInterface(vkDevices);
	
	BuildDRRResources();
	
	vkInterface->FillVertexBuffer(int(VertexBuffer::XRAY), (void *)triangleVertices, sizeof(triangleVertices));
	vkInterface->FillIndexBuffer(0, triangleIndices, indicesN);
	
//	vkInterface->FillVertexBuffer(int(VertexBuffer::DRR), (void *)testTriangleVertices, sizeof(testTriangleVertices));
}

void MainPanel::OnPaint(wxPaintEvent &event){
	DrawFrame();
}
void MainPanel::OnResize(wxSizeEvent &event){
	xRayTransform.viewSize = (vec<2>){float(event.GetSize().x), float(event.GetSize().y)};
	vkInterface->FramebufferResizeCallback();
	DrawFrame();
}
void MainPanel::OnEraseBG(wxEraseEvent &event){
	
}
void MainPanel::OnPaintException(const std::string &message){
	wxMessageBox(message, "Vulkan Error");
	wxTheApp->ExitMainLoop();
}
void MainPanel::OnMouseWheel(wxMouseEvent &event){
	if(!currentXRay) return;
	const vec<2> posNoTranslation = (static_cast<vec<2>>((vec<2, int>){event.GetX(), event.GetY()}) - 0.5f * xRayTransform.viewSize) / xRayTransform.zoom;
	float scale = std::pow(1.0f + zoomSensitivity, static_cast<float>(event.GetWheelRotation()));
	const float prevZoom = xRayTransform.zoom;
	xRayTransform.zoom *= scale;
	if(xRayTransform.zoom < 0.25f){
		xRayTransform.zoom = 0.25f;
		scale = xRayTransform.zoom / prevZoom;
	}
	xRayTransform.viewPos += (1.0f - 1.0f / scale) * posNoTranslation;
	DrawFrame();
}
void MainPanel::OnMouse(wxMouseEvent &event){
	static std::optional<vec<2, int>> positionPrev {};
	
	const vec<2, int> position = vec<2, int>{event.GetX(), event.GetY()};
	
	if(event.ButtonDown()){
		positionPrev = position;
		return;
	}
	
	if(event.Dragging()){
		if(!positionPrev) return;
		
		const vec<2> delta = static_cast<vec<2>>(position - positionPrev.value());
		
		switch(mouseMode){
			case MouseMode::WINDOWING:
				SetXRayWindowingConstants(xRayWindowCentre - 32 * delta.y, xRayWindowWidth + 32 * delta.x);
				break;
			case MouseMode::VIEW_POS:
				xRayTransform.viewPos -= delta / xRayTransform.zoom;
				DrawFrame();
				break;
			default:
				throw std::runtime_error("Unhandled mouse mode");
				break;
		}
		
		positionPrev = position;
		return;
	}

}

void MainPanel::UpdateUBOs(){
	if(currentCT && currentXRay){
		VolumeRendering::CalculatePipelineData(currentXRay.value(),
											   currentCT.value(),
											   (vec<2>){0.0f, 0.0f},
											   (vec<3>){0.0f, 0.0f, 0.0f},
											   mat<4, 4>::Identity(),
											   100,
											   60'000.0f,
											   30'000.0f,
											   vkInterface->GetUniformBufferObjectPointer<VolumeRendering::SharedShaderData>(int(UBO::DRR_SHARED)),
											   vkInterface->GetUniformBufferObjectPointer<VolumeRendering::VertexShaderData>(int(UBO::DRR_VERT)),
											   vkInterface->GetUniformBufferObjectPointer<VolumeRendering::FragmentShaderData>(int(UBO::DRR_FRAG)));
	}
}

void MainPanel::DrawFrame(){
	
	UpdateUBOs();
	
	std::vector<VkClearValue> clearVals = {
		{
			.color = {{0.0f, 0.0f, 0.0f, 0.0f}}
		},
		{
			.depthStencil = {1.0f, 0}
		}
	};
	
	if(vkInterface->BeginFrame()){
		
		vkInterface->CmdBeginBufferedRenderPass(int(BufferedRenderPass::DRR), VK_SUBPASS_CONTENTS_INLINE, clearVals);
		if(currentCT){
			vkInterface->GP(int(GraphicsPipeline::DRR)).Bind();
			vkInterface->GP(int(GraphicsPipeline::DRR)).BindDescriptorSets(0, 1);
			vkInterface->CmdBindVertexBuffer(0, int(VertexBuffer::XRAY));
			vkInterface->CmdBindIndexBuffer(0, VK_INDEX_TYPE_UINT32);
			vkInterface->CmdDrawIndexed(indicesN);
		}
		vkInterface->CmdEndRenderPass();
		
		vkInterface->BeginFinalRenderPass({{0.0f, 0.0f, 0.0f, 1.0f}});
		
		xRayWindowing.haveXRay = currentXRay.has_value();
		
		vkInterface->GP(int(GraphicsPipeline::MAIN)).Bind();
		vkInterface->GP(int(GraphicsPipeline::MAIN)).BindDescriptorSets(0, 2);
		vkInterface->GP(int(GraphicsPipeline::MAIN)).CmdPushConstants(0, &xRayTransform);
		vkInterface->GP(int(GraphicsPipeline::MAIN)).CmdPushConstants(1, &xRayWindowing);
		vkInterface->CmdBindVertexBuffer(0, int(VertexBuffer::XRAY));
		vkInterface->CmdBindIndexBuffer(0, VK_INDEX_TYPE_UINT32);
		vkInterface->CmdDrawIndexed(indicesN);
		
		vkInterface->EndFinalRenderPassAndFrame();
	}
}

void MainPanel::DefaultXRayWindowingConstants(){
	if(!currentXRay) return;
	
	const int64_t valueMax = (1 << (8 * currentXRay->imageDepth));
	
	SetXRayWindowingConstants(valueMax / 2, valueMax);
}
void MainPanel::SetXRayWindowingConstants(int64_t centre, int64_t width){
	if(!currentXRay) return;
	
	const int64_t valueMax = (1 << (8 * currentXRay->imageDepth));
	
	if(centre < 1) centre = 1;
	else if(centre > valueMax - 1) centre = valueMax - 1;
	
	if(width < 2) width = 2;
	
	if(constrainXRayWindowing){
		if(width > valueMax){
			width = valueMax;
			centre = valueMax / 2;
		} else {
			if(centre - width / 2 < 0){
				centre = width / 2;
			}
			if(centre + width / 2 > valueMax){
				centre = valueMax - width / 2;
			}
		}
	}
	
	const float fMaxInverse = 1.0f / static_cast<float>(valueMax - 1);
	xRayWindowing.windowCentre = static_cast<float>(centre) * fMaxInverse;
	xRayWindowing.windowWidth = static_cast<float>(width) * fMaxInverse;
	
	xRayWindowCentre = centre;
	xRayWindowWidth = width;
	
	frameParent->GetViewPanel()->SetWindowingInfo(xRayWindowCentre, xRayWindowWidth);
	
	DrawFrame();
}

void MainPanel::LoadXRay(Data::XRay xRay){
	vkInterface->BuildDataImage(int(Image::XRAY), (EVK::DataImageBlueprint){
		.data = xRay.data.data(),
		xRay.size.x,
		xRay.size.y,
		uint32_t(xRay.imageDepth * xRay.size.x),
		Data::MonochromeVKFormatFromImageDepth(xRay.imageDepth),
		false
	});
	xRayTransform.xraySize = static_cast<vec<2>>(xRay.size);
	BuildDRRResources();
	vkInterface->GP(int(GraphicsPipeline::MAIN)).UpdateDescriptorSets(); // all descriptor sets
	xRayTransform.viewPos = (vec<2>){0.0f, 0.0f};
	xRayTransform.zoom = 1.0f;
	currentXRay = xRay;
	DefaultXRayWindowingConstants();
	DrawFrame();
}

void MainPanel::LoadCT(Data::CT ct){
	vkInterface->Build3DDataImage(int(Image::CT), (EVK::Data3DImageBlueprint){
		.data = ct.data.data(),
		.width = ct.size.x,
		.height = ct.size.y,
		.depth = ct.size.z,
		.pitch = uint32_t(ct.size.x * ct.imageDepth),
		.format = Data::MonochromeVKFormatFromImageDepth(ct.imageDepth)
	});
								   
	vkInterface->GP(int(GraphicsPipeline::DRR)).UpdateDescriptorSets();
	currentCT = ct;
	DrawFrame();
}

void MainPanel::BuildDRRResources(){
	const vec<2, uint32_t> size = currentXRay ? static_cast<vec<2, uint32_t>>(currentXRay->size) : DEFAULT_DRR_SIZE;
	
	vkInterface->AllocateOrResizeBufferedRenderPass(int(BufferedRenderPass::DRR), size);
	
	vkInterface->GP(int(GraphicsPipeline::MAIN)).UpdateDescriptorSets(1); // only the descriptor set with the DRR
}

void MainPanel::ConstructVkDevices(){
	vkDevices = std::make_shared<EVK::Devices>("Volume_Renderer2", GetRequiredExtensions(),
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
std::shared_ptr<EVK::Interface> MainPanel::ConstructVkInterface(std::shared_ptr<EVK::Devices> vkDevices){
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
	
	std::vector<EVK::DescriptorSetBlueprint> mainDSs = {
		{
			(EVK::DescriptorBlueprint){
				.type = EVK::DescriptorType::textureImage,
				.binding = 0,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.indicesExtra = {int(Image::XRAY)},
			}
		},
		{
			(EVK::DescriptorBlueprint){
				.type = EVK::DescriptorType::textureImage,
				.binding = 0,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.indicesExtra = {int(Image::DRR)}
			},
			(EVK::DescriptorBlueprint){
				.type = EVK::DescriptorType::textureSampler,
				.binding = 1,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.indicesExtra = {int(Sampler::MAIN)}
			}
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
		.pipelineBlueprint.descriptorSetBlueprints = mainDSs,
		.pipelineBlueprint.pushConstantRanges = {
			(VkPushConstantRange){
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.offset = 0,
				.size = sizeof(PCS_XRay_Vert)
			},
			(VkPushConstantRange){
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.offset = sizeof(PCS_XRay_Vert),
				.size = sizeof(PCS_XRay_Frag)
			}
		},
		.shaderStageCIs = mainShaderStages,
		.vertexInputStateCI = Vertex::attributeInfo,
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
	
	EVK::DescriptorSetBlueprint drrDs = {
		EVK::DescriptorBlueprint{
			.type = EVK::DescriptorType::UBO,
			.binding = 0,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = {int(UBO::DRR_SHARED)}
		},
		EVK::DescriptorBlueprint{
			.type = EVK::DescriptorType::UBO,
			.binding = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.indicesExtra = {int(UBO::DRR_VERT)}
		},
		EVK::DescriptorBlueprint{
			.type = EVK::DescriptorType::UBO,
			.binding = 2,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = {int(UBO::DRR_FRAG)}
		},
		EVK::DescriptorBlueprint{
			.type = EVK::DescriptorType::combinedImageSampler,
			.binding = 3,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = {int(Image::CT)},
			.indicesExtra2 = {int(Sampler::THREED)}
		}
	};
	
	std::vector<VkPipelineShaderStageCreateInfo> drrShaderStages = shaderStages;
	drrShaderStages[0].module = vkDevices->CreateShaderModule("drr.vert.spv");
	drrShaderStages[1].module = vkDevices->CreateShaderModule("drr.frag.spv");
	colourBlending.attachmentCount = 1;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	dynamicState.dynamicStateCount = 2;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	rasterizer.cullMode = VK_CULL_MODE_NONE;//VK_CULL_MODE_BACK_BIT;
	rasterizer.depthBiasEnable = VK_FALSE;
	EVK::GraphicsPipelineBlueprint pbDrr = {
		.pipelineBlueprint.descriptorSetBlueprints = {drrDs},
		.pipelineBlueprint.pushConstantRanges = {},
		.shaderStageCIs = drrShaderStages,
		.vertexInputStateCI = Vertex::attributeInfo,
//		.vertexInputStateCI = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 0, nullptr, 0, nullptr},
		.rasterisationStateCI = rasterizer,
		.multisampleStateCI = multisampling,
		.colourBlendStateCI = colourBlending,
		.depthStencilStateCI = depthStencil,
		.dynamicStateCI = dynamicState,
		.bufferedRenderPassIndex = {int(BufferedRenderPass::DRR)},
		.layeredBufferedRenderPassIndex = {},
		.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	};
	
	EVK::InterfaceBlueprint nvi {*vkDevices};
	
	nvi.graphicsPipelinesN = int(GraphicsPipeline::_COUNT_);
	nvi.computePipelinesN = 0;
	nvi.uniformBufferObjectsN = int(UBO::_COUNT_);
	nvi.storageBufferObjectsN = 0;
	
	VkSamplerCreateInfo samplerInfoMain{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = vkDevices->GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy,
		.unnormalizedCoordinates = VK_FALSE, // 'VK_TRUE' would mean texture coordinates are (0, texWidth), (0, texHeight)
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.mipLodBias = 0.0f, // Optional
		.minLod = 0.0f, // Optional
		.maxLod = 20.0f // max level of detail (miplevels)
	};
	VkSamplerCreateInfo samplerInfo3D{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy = 1.0f,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_NEVER,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.mipLodBias = 0.0f, // Optional
		.minLod = 0.0f, // Optional
		.maxLod = 0.0f, // max level of detail (miplevels)
		.unnormalizedCoordinates = VK_FALSE // 'VK_TRUE' would mean texture coordinates are (0, texWidth), (0, texHeight)
	};
	nvi.samplerBlueprints = {samplerInfoMain, samplerInfo3D};
	
	nvi.bufferedRenderPassesN = int(BufferedRenderPass::_COUNT_);
	nvi.layeredBufferedRenderPassesN = 0;
	nvi.vertexBuffersN = int(VertexBuffer::_COUNT_);
	nvi.indexBuffersN = 1;
	nvi.imagesN = int(Image::_COUNT_);
	
	std::shared_ptr<EVK::Interface> ret = std::make_shared<EVK::Interface>(nvi);
	
	ret->BuildUBO(int(UBO::DRR_SHARED), EVK::UniformBufferObjectBlueprint{
		.size = sizeof(VolumeRendering::SharedShaderData),
		.dynamicRepeats = std::optional<int>()
	});
	ret->BuildUBO(int(UBO::DRR_VERT), EVK::UniformBufferObjectBlueprint{
		.size = sizeof(VolumeRendering::VertexShaderData),
		.dynamicRepeats = std::optional<int>()
	});
	ret->BuildUBO(int(UBO::DRR_FRAG), EVK::UniformBufferObjectBlueprint{
		.size = sizeof(VolumeRendering::FragmentShaderData),
		.dynamicRepeats = std::optional<int>()
	});
	
	// DRR buffered render pass
	VkAttachmentDescription colourAttachment{
		.format = DRR_IMAGE_FORMAT,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};
	VkAttachmentReference colourAttachmentRef{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	VkSubpassDescription subpass{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colourAttachmentRef
	};
	
	// Use subpass dependencies for layout transitions
	VkSubpassDependency bDependencies[2];
	bDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	bDependencies[0].dstSubpass = 0;
	bDependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	bDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	bDependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	bDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	bDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	bDependencies[1].srcSubpass = 0;
	bDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	bDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	bDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	bDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	bDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	bDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	
	// texture image
	VkImageCreateInfo drrImageCI = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent = {
			.width = DEFAULT_DRR_SIZE.x,
			.height = DEFAULT_DRR_SIZE.y,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.format = DRR_IMAGE_FORMAT,
		.tiling = VK_IMAGE_TILING_OPTIMAL, // VK_IMAGE_TILING_LINEAR for row-major order if we want to access texels in the memory of the image
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	ret->BuildTextureImage(int(Image::DRR), {drrImageCI, VK_IMAGE_VIEW_TYPE_2D, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT});
	
	ret->BuildBufferedRenderPass(int(BufferedRenderPass::DRR), (EVK::BufferedRenderPassBlueprint){
		.renderPassCI = (VkRenderPassCreateInfo){
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = &colourAttachment,
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 2,
			.pDependencies = bDependencies
		},
		.targetTextureImageIndices = {int(Image::DRR)},
		.resizeWithSwapChain = false
	});
	
	ret->BuildGraphicsPipeline(int(GraphicsPipeline::MAIN), pbMain);
	ret->BuildGraphicsPipeline(int(GraphicsPipeline::DRR), pbDrr);
	
	return ret;
}

std::vector<const char*> MainPanel::GetRequiredExtensions() {
#ifdef _WIN32
	std::vector<const char*> extensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
#elif defined(__APPLE__)
	std::vector<const char*> extensions = { "VK_KHR_surface", "VK_EXT_metal_surface", "VK_KHR_portability_enumeration"};
#else
	std::vector<const char*> extensions = { "VK_KHR_surface", "VK_KHR_wayland_surface", "VK_KHR_xlib_surface" };
#endif
	return extensions;
}
VkSurfaceKHR MainPanel::CreateFramebufferSurface(VkInstance instance, struct WindowHandleInfo& windowInfo){
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

void MainPanel::gui_initHandleContextFromWxWidgetsWindow(WindowHandleInfo& handleInfoOut, class wxWindow* wxw){
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
