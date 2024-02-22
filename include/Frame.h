#pragma once

#include "wx/wx.h"
#include "wx/wxprec.h"

#include "ControlPanel.h"
#include "MainPanel.h"
#include "ViewPanel.h"
#include "Data.h"

class Frame : public wxFrame {
private:
	struct XRayGaussianManager;
	
public:
	Frame(wxWindow *parent, const wxString &title, const wxPoint &pos, const wxSize &size);
	virtual ~Frame();
	
	void Idle();
	
	bool CheckValidPanel() const { return bool(mainPanel); }
	
	MainPanel *GetMainPanel() const { return mainPanel; }
	ViewPanel *GetViewPanel() const { return viewPanel; }
	
	void OnOpenXray(wxCommandEvent &event);
	void UpdateXRayGaussian(float newSigma);
	
	void OnOpenCT(wxCommandEvent &event);
	
	const XRayGaussianManager &GetXRayGaussianManager() const { return xRayGaussianManager; }
	
	Data::params_t &Parameters(){ return parameters; }
	
private:
	wxBoxSizer *mainSizer;
	ControlPanel *controlPanel;
	wxBoxSizer *rightSizer;
	MainPanel *mainPanel;
	ViewPanel *viewPanel;
	
	std::optional<Data::XRay> xRay {};
	std::optional<Data::CT> ct {};
	Data::params_t parameters {0.0f, 0.0f, 0.0f, 0.0f, -131.164f, 0.0f, 0.0f, 0.0f};
	
	//void OnResize(wxSizeEvent& event);
	void OnCreate(wxPaintEvent &event);
	
	bool _first_paint_call;
	
	void OnClose(wxCloseEvent &event);
	void OnQuit(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);
	
	struct XRayGaussianManager {
		static constexpr int precision = 1000;
		
		float low = 0.0f;
		float high = 5.0f;
		float dfault = 1.78f;
		
		int FloatToInt(float f) const {
			return int(roundf(float(precision) * (f - low) / (high - low)));
		}
		float IntToFloat(int i) const {
			return low + (high - low) * float(i) / float(precision);
		}
		int IntDefault() const {
			return IntToFloat(dfault);
		}
	};
	XRayGaussianManager xRayGaussianManager {};
	
    DECLARE_EVENT_TABLE()
};

