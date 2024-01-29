#include <chrono>
#include <iostream>
#include <sstream>

#include <wx/wxprec.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "Frame.h"
#include "VulkanException.h"

#include "App.h"

#ifdef __linux__
#include <X11/Xlib.h>
#endif

// #pragma warning(disable: 28251)

/*
#ifdef _UNICODE
#ifdef _DEBUG
#pragma comment(lib, "wxbase32ud.lib")
#else
#pragma comment(lib, "wxbase32u.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "wxbase32d.lib")
#else
#pragma comment(lib, "wxbase32.lib")
#endif
#endif
*/

Frame *mainFrame;

App::App() {
#ifdef __linux__
    //setenv("GDK_BACKEND", "x11", 1);
    //XInitThreads();
#endif
}

App::~App() {}

bool App::OnInit(){
    try {
    
        mainFrame = new Frame(NULL, "Volume_Renderer2", wxDefaultPosition, wxSize(1280, 1020));
        
    } catch(VulkanException &ve) {
        std::string status = ve.GetStatus();
        std::stringstream ss;
        ss << ve.what() << "\n" << status;
        wxMessageBox(ss.str(), "Vulkan Error");
        return false;
    } catch (std::runtime_error &err) {
        std::stringstream ss;
        ss << "Error encountered trying to create the Vulkan canvas:\n";
        ss << err.what();
        wxMessageBox(ss.str(), "Vulkan Error");
        return false;
    }
    
    mainFrame->SetMinSize(wxSize(400,400));
    mainFrame->Show(true);
    
    return true;
}

void App::OnIdle(wxIdleEvent &event) {
//     static unsigned long long last_time = 0;
//     unsigned long long this_time;
//     static int fps_update_counter = 0;
//     float time_diff;
// 
//     using namespace std::chrono;
//     // Update frames-per-second display
//     this_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
// 
//     if (last_time == 0) last_time = this_time;
//     else {
//         fps_update_counter++;
//         time_diff = (this_time - last_time) / 1.0e6;
//         if (time_diff >= FPS_UPDATE_INTERVAL) {
//             mainFrame->fps = (int)(fps_update_counter / time_diff);
//             mainFrame->SetStatusText(wxString::Format("%d frames per second", mainFrame->fps), 1);
//             mainFrame->tl_canvas->fps = mainFrame->fps;
//             last_time = this_time;
//             fps_update_counter = 0;
//         }
//     }

	mainFrame->Idle();
    wxWakeUpIdle();
}

void App::OnXLaunchIdle(wxIdleEvent &event)
{
    if (mainFrame->CheckValidPanel()) {
    	mainFrame->Refresh();
    	mainFrame->Update();
    	wxWakeUpIdle();
    }
}

wxIMPLEMENT_APP(App);
