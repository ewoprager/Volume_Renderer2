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

	SetMinSize(wxSize(400,400));
	Show(true);
	
    // Lay out the canvases and controls
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(wxID_ABOUT, "&About");
    fileMenu->Append(wxID_EXIT, "&Quit");
	wxMenuItem *openXrayItem = fileMenu->Append(wxID_ANY, "&Load X-Ray DICOM");
		
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    SetMenuBar(menuBar);
		
    wxStatusBar *statusBar = CreateStatusBar(2);
    int widths[2] = {-2, -1};
    statusBar->SetStatusWidths(2, widths);
    int styles[2] = {wxSB_SUNKEN, wxSB_SUNKEN};
    statusBar->SetStatusStyles(2, styles);
	
	wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(mainSizer);
	
	controlPanel = new ControlPanel(this, wxID_ANY);
	mainSizer->Add(controlPanel, 0, wxEXPAND);
	
	wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);
	
	mainPanel = new MainPanel(this, wxID_ANY);
	rightSizer->Add(mainPanel, 1, wxEXPAND);
	
	viewPanel = new ViewPanel(this, wxID_ANY);
	rightSizer->Add(viewPanel, 0, wxEXPAND);
	
	mainSizer->Add(rightSizer, 1, wxEXPAND);

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
		controlPanel->SetXRayInfo(xRay.value());
		
		UpdateXRayGaussian(xRayGaussianManager.dfault);
	}
}
void Frame::OnOpenCT(wxCommandEvent &event){
	wxDirDialog openDirDialog{this, _("Open an CT DICOM file"), "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST};
	
	if (openDirDialog.ShowModal() == wxID_CANCEL)
		return;
	
	ct = Data::DICOM::LoadCT(openDirDialog.GetPath());
	if(ct){
		controlPanel->SetCTInfo(ct.value());
		
		mainPanel->LoadCT(ct.value());
	}
}
void Frame::UpdateXRayGaussian(float newSigma){
	if(!xRay) return;
	
	Data::XRay copy = xRay.value();
	Data::GaussianBlur(copy, newSigma);
	mainPanel->LoadXRay(copy);
}

void Frame::OnClose(wxCloseEvent& event){
    Destroy();
    exit(0); // a workaround for a wxWidgets bug, should be able to remove this once the wxWidgets bug is fixed
}

void Frame::OnQuit(wxCommandEvent& event){
    Close(true);
}

void Frame::OnAbout(wxCommandEvent& event){
    wxMessageDialog about(this, "Volume renderer and 2D/3D registration tool for radiographic cochlear implant data\nEdmund Prager - 2024", "About Volume_Renderer2", wxICON_INFORMATION | wxOK);
    about.ShowModal();
}


