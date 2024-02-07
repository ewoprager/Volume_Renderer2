#include <wx/file.h>
#include <wx/log.h>

#include "App.h"
#include "ImProc.h"

#include "Frame.h"

BEGIN_EVENT_TABLE(Frame, wxFrame)
    EVT_MENU(wxID_EXIT, Frame::OnQuit)
    EVT_MENU(wxID_ABOUT, Frame::OnAbout)
//    EVT_COMMAND_SCROLL(VA_SLIDER_ID, Frame::OnVASlider)
//    EVT_CHECKBOX(SLICE_CHECKBOX_ID, Frame::OnSlice)
    EVT_CLOSE(Frame::OnClose)
END_EVENT_TABLE()

Frame::Frame(wxWindow *parent, const wxString &title, const wxPoint &pos, const wxSize &size)
: wxFrame(parent, wxID_ANY, title, pos, size) {

    // Lay out the canvases and controls
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(wxID_ABOUT, "&About");
    fileMenu->Append(wxID_EXIT, "&Quit");
	wxMenuItem *openXrayItem = fileMenu->Append(wxID_ANY, "&Open X-Ray");
		
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    SetMenuBar(menuBar);
		
    wxStatusBar *statusBar = CreateStatusBar(2);
    int widths[2] = {-2, -1};
    statusBar->SetStatusWidths(2, widths);
    int styles[2] = {wxSB_SUNKEN, wxSB_SUNKEN};
    statusBar->SetStatusStyles(2, styles);
	
	mainSizer = std::make_shared<wxBoxSizer>(wxHORIZONTAL);
	SetSizer(mainSizer.get());
	
	controlPanel = std::make_shared<ControlPanel>(this, -1);
	controlPanel->SetScrollRate(10, 10);
	controlPanel->SetMinSize(wxSize{210, -1});
	mainSizer->Add(controlPanel.get(), 0, wxEXPAND);
	
	rightSizer = std::make_shared<wxBoxSizer>(wxVERTICAL);
	
	mainPanel = std::make_shared<MainPanel>(this, wxID_ANY);
	rightSizer->Add(mainPanel.get(), 1, wxEXPAND);
	
	viewPanel = std::make_shared<ViewPanel>(mainPanel.get(), this, wxID_ANY);
	rightSizer->Add(viewPanel.get(), 0, wxEXPAND);
	
	mainSizer->Add(rightSizer.get(), 1, wxEXPAND);

    Bind(wxEVT_PAINT, &Frame::OnCreate, this);
	Bind(wxEVT_MENU, &Frame::OnOpenXray, this, openXrayItem->GetId());
}


Frame::~Frame() {}

//void VulkanWindow::OnResize(wxSizeEvent& event)
//{
//    wxSize clientSize = GetClientSize();
//    //m_canvas->SetSize(clientSize);
//}

void Frame::OnCreate(wxPaintEvent &event) {
	
	SendSizeEvent();
	
	mainPanel->DrawFrame();
	
    Unbind(wxEVT_PAINT, &Frame::OnCreate, this);

#ifdef __linux__
    WindowHandleInfo handle;
    tl_canvas->gui_initHandleContextFromWxWidgetsWindow(handle, this);
    if (handle.backend == WindowHandleInfo::Backend::X11) {
        wxTheApp->Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(App::OnXLaunchIdle));
    }
#endif
}

void Frame::Idle(){
	mainPanel->DrawFrame();
}

void Frame::OnOpenXray(wxCommandEvent &event){
	wxFileDialog openFileDialog{this, _("Open an X-Ray DICOM file"), "", "", "DICOM files (*.dcm)|*.dcm", wxFD_OPEN | wxFD_FILE_MUST_EXIST};
	
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return;
	
	xRay = Data::DICOM::LoadXRay(openFileDialog.GetPath());
	if(xRay){
		std::cout << "XRay loaded!\nSize = [" << xRay->size.x << " x " << xRay->size.y << "]\nPixel spacing = " << xRay->pixelSpacing << "\nSource distance = " << xRay->sourceDistance << "\nImage depth = " << xRay->imageDepth << "\n";
		
		Data::XRay copy = xRay.value();
		Data::GaussianBlur(copy);
		mainPanel->LoadXRay(copy);
	}
}

void Frame::OnClose(wxCloseEvent& event)
// Called when application is closed down (including window manager close)
{
    Destroy();
    exit(0); // a workaround for a wxWidgets bug, should be able to remove this once the wxWidgets bug is fixed
}

void Frame::OnQuit(wxCommandEvent& event)
// Callback for the "Quit" menu item
{
    Close(true);
}

void Frame::OnAbout(wxCommandEvent& event)
// Callback for the "About" menu item
{
    wxMessageDialog about(this, "Volume renderer and 2D/3D registration tool for radiographic cochlear implant data\nEdmund Prager - 2024", "About Volume_Renderer2", wxICON_INFORMATION | wxOK);
    about.ShowModal();
}
// 
// void VulkanWindow::OnROSlider(wxScrollEvent& event)
// // Callback for the reslice opacity slider
// {
//     reslice_alpha = event.GetPosition();
//     construct_data_slice_image();
//     construct_xz_reslice();
//     construct_yz_reslice();
//     tl_canvas->slice_opacity = reslice_alpha/10.0f;
//     tl_canvas->drawFrame();
// }

//void VulkanWindow::OnSlice(wxCommandEvent& event)
// // Callback for the slice/reslice checkbox
// {
//     show_reslices = event.IsChecked();
//     tl_canvas->show_slices = show_reslices;
//     tl_canvas->drawFrame();
// }
