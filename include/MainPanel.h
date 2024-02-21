#pragma once

#include "wx/wx.h"
#include <evk/Base.hpp>

#include "guistructs.h"

#include "Header.h"
#include "Data.h"

class MainPanel : public wxPanel {
public:
	MainPanel( Frame *_frameParent, wxWindowID id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0, const wxString &name = "PanelName");
	
	void DrawFrame();
	
	void LoadXRay(Data::XRay xRay);
	void LoadCT(Data::CT ct);
	
	enum class MouseMode {WINDOWING, VIEW_POS, _COUNT_};
	void SetMouseMode(MouseMode to){ mouseMode = to; }
	void SetConstrainWindowing(bool value){
		constrainXRayWindowing = value;
		if(constrainXRayWindowing) SetXRayWindowingConstants(xRayWindowCentre, xRayWindowWidth);
	}
	
	void DefaultXRayWindowingConstants();
	
private:
	Frame *frameParent;
	
	std::shared_ptr<EVK::Devices> vkDevices {};
	std::shared_ptr<EVK::Interface> vkInterface {};
	enum class Image {
		XRAY, DRR, CT, _COUNT_
	};
	enum class BufferedRenderPass {
		DRR, _COUNT_
	};
	enum class GraphicsPipeline {
		MAIN, DRR, _COUNT_
	};
	enum class VertexBuffer {
		XRAY, DRR, _COUNT_
	};
	enum class UBO {
		DRR_SHARED, DRR_VERT, DRR_FRAG, _COUNT_
	};
	enum class Sampler {
		MAIN, THREED, _COUNT_
	};
	struct PCS_XRay_Vert {
		vec<2> viewPos;
		vec<2> xraySize;
		vec<2> viewSize;
		float zoom;
	};
	struct PCS_XRay_Frag {
		float windowCentre;
		float windowWidth;
		bool haveXRay;
	};	
	static constexpr VkFormat DRR_IMAGE_FORMAT = VK_FORMAT_R16_UNORM;
	static constexpr vec<2, uint32_t> DEFAULT_DRR_SIZE = (vec<2, uint32_t>){1024, 1024};
	
	std::optional<Data::XRay> currentXRay {};
	std::optional<Data::CT> currentCT {};
	
	void BuildDRRResources();
	
	PCS_XRay_Vert xRayTransform;
	int64_t xRayWindowCentre;
	int64_t xRayWindowWidth;
	PCS_XRay_Frag xRayWindowing;
	bool constrainXRayWindowing = false;
	void SetXRayWindowingConstants(int64_t centre, int64_t width);
	float zoomSensitivity = 8.e-4f;
	MouseMode mouseMode = MouseMode::WINDOWING;
	/*
	 origin is in centre of Xray
	 x-positive right
	 y-positive down
	 normal scale is that of Xray pixels
	 */
	
	void OnPaint(wxPaintEvent &event);
	void OnResize(wxSizeEvent &event);
	void OnEraseBG(wxEraseEvent &event);
	void OnPaintException(const std::string &message);
	void OnMouseWheel(wxMouseEvent &event);
	void OnMouse(wxMouseEvent &event);
	
	void UpdateUBOs();
	
	void ConstructVkDevices();
	static std::shared_ptr<EVK::Interface> ConstructVkInterface(std::shared_ptr<EVK::Devices> vkDevices);
	static std::vector<const char*> GetRequiredExtensions();
	WindowInfo g_window_info {};
	static VkSurfaceKHR CreateFramebufferSurface(VkInstance instance, struct WindowHandleInfo& windowInfo);
#ifdef _WIN32
	static VkSurfaceKHR CreateWinSurface(VkInstance instance, HWND hwindow);
#endif
#ifdef __linux__
	static VkSurfaceKHR CreateWaylandSurface(VkInstance instance, wl_display* wl_display, wl_surface* wl_surface);
	static VkSurfaceKHR CreateXlibSurface(VkInstance instance, Display* dpy, Window window);
	std::unique_ptr<wxWlSubsurface> m_subsurface;
#endif
	static void gui_initHandleContextFromWxWidgetsWindow(WindowHandleInfo &handleInfoOut, class wxWindow *wxw);
	
	DECLARE_EVENT_TABLE()
};
