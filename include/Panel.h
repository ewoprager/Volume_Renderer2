#pragma once

#include "wx/wx.h"
#include <evk/Base.hpp>
#include <mattresses.h>

#include "guistructs.h"

#include "Data.h"

struct PCS_XRay {
	vec<2> viewPos;
	vec<2> xraySize;
	vec<2> viewSize;
	float zoom;
};

class Panel : wxPanel {
public:
	Panel(wxWindow *pParent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0, const wxString &name = "PanelName");
	
	void DrawFrame();
	
	void LoadXRay(Data::DICOM::XRay xRay);
	
private:
	std::shared_ptr<EVK::Devices> vkDevices {};
	std::shared_ptr<EVK::Interface> vkInterface {};
	
	bool xRayLoaded = false;
	PCS_XRay xRayTransform;
	float zoomSensitivity = 8.e-4f;
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
	
	std::shared_ptr<EVK::Devices> ConstructVkDevices();
	std::shared_ptr<EVK::Interface> ConstructVkInterface();
	std::vector<const char*> GetRequiredExtensions();
	WindowInfo g_window_info {};
	VkSurfaceKHR CreateFramebufferSurface(VkInstance instance, struct WindowHandleInfo& windowInfo);
#ifdef _WIN32
	VkSurfaceKHR CreateWinSurface(VkInstance instance, HWND hwindow);
#endif
#ifdef __linux__
	VkSurfaceKHR CreateWaylandSurface(VkInstance instance, wl_display* wl_display, wl_surface* wl_surface);
	VkSurfaceKHR CreateXlibSurface(VkInstance instance, Display* dpy, Window window);
	std::unique_ptr<wxWlSubsurface> m_subsurface;
#endif
	void gui_initHandleContextFromWxWidgetsWindow(WindowHandleInfo &handleInfoOut, class wxWindow *wxw);
	
	DECLARE_EVENT_TABLE()
};
