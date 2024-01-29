#pragma once

#include <atomic>

#ifdef __linux__
#include <gdk/gdkx.h>
#include <gdk/gdkwayland.h>
#include <wayland-client.h>
#endif

struct WindowHandleInfo
{
#ifdef _WIN32
	std::atomic<HWND> hwnd;
#elif defined(__linux__)
	bool is_wayland;
	enum class Backend
	{
		X11,
		WAYLAND,
	} backend;
    // XLIB
    Display* xlib_display{};
    Window xlib_window{};

	// XCB (not used by GTK so we cant retrieve these without making our own window)
	//xcb_connection_t* xcb_con{};
	//xcb_window_t xcb_window{};

	// Wayland
	wl_display* wayland_display{};
	wl_surface* wayland_surface{};
#else
	void* handle;
#endif
};

struct WindowInfo
{
	std::atomic_bool app_active; // our app is active/has focus

	std::atomic_int32_t width, height; 	// client size of main window

	std::atomic_bool pad_open; // if separate pad view is open
	std::atomic_int32_t pad_width, pad_height; 	// client size of pad window

	std::atomic_bool pad_maximized = false;
	std::atomic_int32_t restored_pad_x = -1, restored_pad_y = -1;
	std::atomic_int32_t restored_pad_width = -1, restored_pad_height = -1;

	std::atomic_bool has_screenshot_request;
	std::atomic_bool is_fullscreen;

	WindowHandleInfo window_main;
	WindowHandleInfo window_pad;

	// canvas
	WindowHandleInfo canvas_main;
	WindowHandleInfo canvas_pad;
   private:

};