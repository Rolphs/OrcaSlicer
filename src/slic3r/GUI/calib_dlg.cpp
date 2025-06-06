#include "calib_dlg.hpp"
#include "GUI_App.hpp"
#include "MsgDialog.hpp"
#include "I18N.hpp"
#include <wx/dcgraph.h>
#include "MainFrame.hpp"
#include "Widgets/DialogButtons.hpp"
#include <string>
namespace Slic3r { namespace GUI {

namespace {

void ParseStringValues(std::string str, std::vector<double> &vec)
{
    vec.clear();
    std::replace(str.begin(), str.end(), ',', ' ');
    std::istringstream inss(str);
    std::copy_if(std::istream_iterator<int>(inss), std::istream_iterator<int>(), std::back_inserter(vec),
                 [](int x){ return x > 0; });
}

}

wxBoxSizer* create_item_checkbox(wxString title, wxWindow* parent, bool* value, CheckBox*& checkbox)
{
    wxBoxSizer* m_sizer_checkbox = new wxBoxSizer(wxHORIZONTAL);

    m_sizer_checkbox->Add(0, 0, 0, wxEXPAND | wxLEFT, 5);

    checkbox = new ::CheckBox(parent);
    m_sizer_checkbox->Add(checkbox, 0, wxALIGN_CENTER, 0);
    m_sizer_checkbox->Add(0, 0, 0, wxEXPAND | wxLEFT, 8);

    auto checkbox_title = new wxStaticText(parent, wxID_ANY, title, wxDefaultPosition, wxSize(-1, -1), 0);
    checkbox_title->SetForegroundColour(wxColour(144, 144, 144));
    checkbox_title->SetFont(::Label::Body_13);
    checkbox_title->Wrap(-1);
    m_sizer_checkbox->Add(checkbox_title, 0, wxALIGN_CENTER | wxALL, 3);

    checkbox->SetValue(true);

    checkbox->Bind(wxEVT_TOGGLEBUTTON, [parent, checkbox, value](wxCommandEvent& e) {
        (*value) = (*value) ? false : true;
        e.Skip();
        });

    return m_sizer_checkbox;
}

PA_Calibration_Dlg::PA_Calibration_Dlg(wxWindow* parent, wxWindowID id, Plater* plater)
    : DPIDialog(parent, id, _L("PA Calibration"), wxDefaultPosition, parent->FromDIP(wxSize(-1, 280)), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), m_plater(plater)
{
    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(v_sizer);
	wxBoxSizer* choice_sizer = new wxBoxSizer(wxHORIZONTAL);

    wxString m_rbExtruderTypeChoices[] = { _L("DDE"), _L("Bowden") };
	int m_rbExtruderTypeNChoices = sizeof(m_rbExtruderTypeChoices) / sizeof(wxString);
	m_rbExtruderType = new wxRadioBox(this, wxID_ANY, _L("Extruder type"), wxDefaultPosition, wxDefaultSize, m_rbExtruderTypeNChoices, m_rbExtruderTypeChoices, 2, wxRA_SPECIFY_COLS);
	m_rbExtruderType->SetSelection(0);
	choice_sizer->Add(m_rbExtruderType, 0, wxALL, 5);
	choice_sizer->Add(FromDIP(5), 0, 0, wxEXPAND, 5);
	wxString m_rbMethodChoices[] = { _L("PA Tower"), _L("PA Line"), _L("PA Pattern") };
	int m_rbMethodNChoices = sizeof(m_rbMethodChoices) / sizeof(wxString);
	m_rbMethod = new wxRadioBox(this, wxID_ANY, _L("Method"), wxDefaultPosition, wxDefaultSize, m_rbMethodNChoices, m_rbMethodChoices, 2, wxRA_SPECIFY_COLS);
	m_rbMethod->SetSelection(0);
	choice_sizer->Add(m_rbMethod, 0, wxALL, 5);

	v_sizer->Add(choice_sizer);

    // Settings
    //
    wxString start_pa_str = _L("Start PA: ");
    wxString end_pa_str = _L("End PA: ");
    wxString PA_step_str = _L("PA step: ");
    wxString sp_accel_str = _L("Accelerations: ");
    wxString sp_speed_str = _L("Speeds: ");
	auto text_size = wxWindow::GetTextExtent(start_pa_str);
	text_size.IncTo(wxWindow::GetTextExtent(end_pa_str));
	text_size.IncTo(wxWindow::GetTextExtent(PA_step_str));
    text_size.IncTo(wxWindow::GetTextExtent(sp_accel_str));
    text_size.IncTo(wxWindow::GetTextExtent(sp_speed_str));
    text_size.x = text_size.x * 1.1;
	wxStaticBoxSizer* settings_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _L("Settings"));

	auto st_size = FromDIP(wxSize(text_size.x, -1));
    auto ti_size = FromDIP(wxSize(140, -1));
    // start PA
    auto start_PA_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto start_pa_text = new wxStaticText(this, wxID_ANY, start_pa_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiStartPA = new TextInput(this, "", "", "", wxDefaultPosition, ti_size, wxTE_CENTRE | wxTE_PROCESS_ENTER);
    m_tiStartPA->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

	start_PA_sizer->Add(start_pa_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    start_PA_sizer->Add(m_tiStartPA, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(start_PA_sizer);

    // end PA
    auto end_PA_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto end_pa_text = new wxStaticText(this, wxID_ANY, end_pa_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiEndPA = new TextInput(this, "", "", "", wxDefaultPosition, ti_size, wxTE_CENTRE | wxTE_PROCESS_ENTER);
    m_tiStartPA->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    end_PA_sizer->Add(end_pa_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    end_PA_sizer->Add(m_tiEndPA, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(end_PA_sizer);

    // PA step
    auto PA_step_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto PA_step_text = new wxStaticText(this, wxID_ANY, PA_step_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiPAStep = new TextInput(this, "", "", "", wxDefaultPosition, ti_size, wxTE_CENTRE | wxTE_PROCESS_ENTER);
    m_tiStartPA->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    PA_step_sizer->Add(PA_step_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    PA_step_sizer->Add(m_tiPAStep, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(PA_step_sizer);

	settings_sizer->Add(create_item_checkbox(_L("Print numbers"), this, &m_params.print_numbers, m_cbPrintNum));
    m_cbPrintNum->SetValue(false);

    wxTextValidator val_list_validator(wxFILTER_INCLUDE_CHAR_LIST);
    val_list_validator.SetCharIncludes(wxString("0123456789,"));

    auto sp_accel_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto sp_accel_text = new wxStaticText(this, wxID_ANY, sp_accel_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiBMAccels = new TextInput(this, "", "", "", wxDefaultPosition, ti_size, wxTE_PROCESS_ENTER);
    m_tiBMAccels->SetToolTip(_L("Comma-separated list of printing accelerations"));
    m_tiBMAccels->GetTextCtrl()->SetValidator(val_list_validator);
    sp_accel_sizer->Add(sp_accel_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sp_accel_sizer->Add(m_tiBMAccels, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(sp_accel_sizer);

    auto sp_speed_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto sp_speed_text = new wxStaticText(this, wxID_ANY, sp_speed_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiBMSpeeds = new TextInput(this, "", "", "", wxDefaultPosition, ti_size, wxTE_PROCESS_ENTER);
    m_tiBMSpeeds->SetToolTip(_L("Comma-separated list of printing speeds"));
    m_tiBMSpeeds->GetTextCtrl()->SetValidator(val_list_validator);
    sp_speed_sizer->Add(sp_speed_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sp_speed_sizer->Add(m_tiBMSpeeds, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(sp_speed_sizer);

    v_sizer->Add(settings_sizer);
	v_sizer->Add(0, FromDIP(10), 0, wxEXPAND, 5);

    auto dlg_btns = new DialogButtons(this, {"OK"});
    v_sizer->Add(dlg_btns , 0, wxEXPAND);

    dlg_btns->GetOK()->Bind(wxEVT_BUTTON, &PA_Calibration_Dlg::on_start, this);

    PA_Calibration_Dlg::reset_params();

    // Connect Events
    m_rbExtruderType->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(PA_Calibration_Dlg::on_extruder_type_changed), NULL, this);
    m_rbMethod->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(PA_Calibration_Dlg::on_method_changed), NULL, this);
    this->Connect(wxEVT_SHOW, wxShowEventHandler(PA_Calibration_Dlg::on_show));
    //wxGetApp().UpdateDlgDarkUI(this);

    Layout();
    Fit();
}

PA_Calibration_Dlg::~PA_Calibration_Dlg() {
    // Disconnect Events
    m_rbExtruderType->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(PA_Calibration_Dlg::on_extruder_type_changed), NULL, this);
    m_rbMethod->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(PA_Calibration_Dlg::on_method_changed), NULL, this);
}

void PA_Calibration_Dlg::reset_params() {
    bool isDDE = m_rbExtruderType->GetSelection() == 0 ? true : false;
    int method = m_rbMethod->GetSelection();

    m_tiStartPA->GetTextCtrl()->SetValue(wxString::FromDouble(0.0));

    switch (method) {
        case 1:
            m_params.mode = CalibMode::Calib_PA_Line;
            m_tiEndPA->GetTextCtrl()->SetValue(wxString::FromDouble(0.1));
            m_tiPAStep->GetTextCtrl()->SetValue(wxString::FromDouble(0.002));
            m_cbPrintNum->SetValue(true);
            m_cbPrintNum->Enable(true);
            m_tiBMAccels->Enable(false);
            m_tiBMSpeeds->Enable(false);
            break;
        case 2:
            m_params.mode = CalibMode::Calib_PA_Pattern;
            m_tiEndPA->GetTextCtrl()->SetValue(wxString::FromDouble(0.08));
            m_tiPAStep->GetTextCtrl()->SetValue(wxString::FromDouble(0.005));
            m_cbPrintNum->SetValue(true);
            m_cbPrintNum->Enable(false);
            m_tiBMAccels->Enable(true);
            m_tiBMSpeeds->Enable(true);
            break;
        default:
            m_params.mode = CalibMode::Calib_PA_Tower;
            m_tiEndPA->GetTextCtrl()->SetValue(wxString::FromDouble(0.1));
            m_tiPAStep->GetTextCtrl()->SetValue(wxString::FromDouble(0.002));
            m_cbPrintNum->SetValue(false);
            m_cbPrintNum->Enable(false);
            m_tiBMAccels->Enable(false);
            m_tiBMSpeeds->Enable(false);
            break;
    }

    if (!isDDE) {
        m_tiEndPA->GetTextCtrl()->SetValue(wxString::FromDouble(1.0));
        
        if (m_params.mode == CalibMode::Calib_PA_Pattern) {
            m_tiPAStep->GetTextCtrl()->SetValue(wxString::FromDouble(0.05));
        } else {
            m_tiPAStep->GetTextCtrl()->SetValue(wxString::FromDouble(0.02));
        }
    }
}

void PA_Calibration_Dlg::on_start(wxCommandEvent& event) { 
    bool read_double = false;
    read_double = m_tiStartPA->GetTextCtrl()->GetValue().ToDouble(&m_params.start);
    read_double = read_double && m_tiEndPA->GetTextCtrl()->GetValue().ToDouble(&m_params.end);
    read_double = read_double && m_tiPAStep->GetTextCtrl()->GetValue().ToDouble(&m_params.step);
    if (!read_double || m_params.start < 0 || m_params.step < EPSILON || m_params.end < m_params.start + m_params.step) {
        MessageDialog msg_dlg(nullptr, _L("Please input valid values:\nStart PA: >= 0.0\nEnd PA: > Start PA\nPA step: >= 0.001)"), wxEmptyString, wxICON_WARNING | wxOK);
        msg_dlg.ShowModal();
        return;
    }

    switch (m_rbMethod->GetSelection()) {
        case 1:
            m_params.mode = CalibMode::Calib_PA_Line;
            break;
        case 2:
            m_params.mode = CalibMode::Calib_PA_Pattern;
            break;
        default:
            m_params.mode = CalibMode::Calib_PA_Tower;
    }

    m_params.print_numbers = m_cbPrintNum->GetValue();
    ParseStringValues(m_tiBMAccels->GetTextCtrl()->GetValue().ToStdString(), m_params.accelerations);
    ParseStringValues(m_tiBMSpeeds->GetTextCtrl()->GetValue().ToStdString(), m_params.speeds);

    m_plater->calib_pa(m_params);
    EndModal(wxID_OK);

}
void PA_Calibration_Dlg::on_extruder_type_changed(wxCommandEvent& event) { 
    PA_Calibration_Dlg::reset_params();
    event.Skip(); 
}
void PA_Calibration_Dlg::on_method_changed(wxCommandEvent& event) { 
    PA_Calibration_Dlg::reset_params();
    event.Skip(); 
}

void PA_Calibration_Dlg::on_dpi_changed(const wxRect& suggested_rect) {
    this->Refresh(); 
    Fit();
}

void PA_Calibration_Dlg::on_show(wxShowEvent& event) {
    PA_Calibration_Dlg::reset_params();
}

// Temp calib dlg
//
enum FILAMENT_TYPE : int
{
    tPLA = 0,
    tABS_ASA,
    tPETG,
    tPCTG,
    tTPU,
    tPA_CF,
    tPET_CF,
    tCustom
};

Temp_Calibration_Dlg::Temp_Calibration_Dlg(wxWindow* parent, wxWindowID id, Plater* plater)
    : DPIDialog(parent, id, _L("Temperature calibration"), wxDefaultPosition, parent->FromDIP(wxSize(-1, 280)), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), m_plater(plater)
{
    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(v_sizer);
    wxBoxSizer* choice_sizer = new wxBoxSizer(wxHORIZONTAL);

    wxString m_rbFilamentTypeChoices[] = { _L("PLA"), _L("ABS/ASA"), _L("PETG"), _L("PCTG"), _L("TPU"), _L("PA-CF"), _L("PET-CF"), _L("Custom") };
    int m_rbFilamentTypeNChoices = sizeof(m_rbFilamentTypeChoices) / sizeof(wxString);
    m_rbFilamentType = new wxRadioBox(this, wxID_ANY, _L("Filament type"), wxDefaultPosition, wxDefaultSize, m_rbFilamentTypeNChoices, m_rbFilamentTypeChoices, 2, wxRA_SPECIFY_COLS);
    m_rbFilamentType->SetSelection(0);
    m_rbFilamentType->Select(0);
    choice_sizer->Add(m_rbFilamentType, 0, wxALL, 5);
    choice_sizer->Add(FromDIP(5), 0, 0, wxEXPAND, 5);
    wxString m_rbMethodChoices[] = { _L("PA Tower"), _L("PA Line") };

    v_sizer->Add(choice_sizer);

    // Settings
    //
    wxString start_temp_str = _L("Start temp: ");
    wxString end_temp_str = _L("End temp: ");
    wxString temp_step_str = _L("Temp step: ");
    auto text_size = wxWindow::GetTextExtent(start_temp_str);
    text_size.IncTo(wxWindow::GetTextExtent(end_temp_str));
    text_size.IncTo(wxWindow::GetTextExtent(temp_step_str));
    text_size.x = text_size.x * 1.5;
    wxStaticBoxSizer* settings_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _L("Settings"));

    auto st_size = FromDIP(wxSize(text_size.x, -1));
    auto ti_size = FromDIP(wxSize(90, -1));
    // start temp
    auto start_temp_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto start_temp_text = new wxStaticText(this, wxID_ANY, start_temp_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiStart = new TextInput(this, std::to_string(230), _L("\u2103"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

    start_temp_sizer->Add(start_temp_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    start_temp_sizer->Add(m_tiStart, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(start_temp_sizer);

    // end temp
    auto end_temp_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto end_temp_text = new wxStaticText(this, wxID_ANY, end_temp_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiEnd = new TextInput(this, std::to_string(190), _L("\u2103"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    end_temp_sizer->Add(end_temp_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    end_temp_sizer->Add(m_tiEnd, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(end_temp_sizer);

    // temp step
    auto temp_step_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto temp_step_text = new wxStaticText(this, wxID_ANY, temp_step_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiStep = new TextInput(this, wxString::FromDouble(5),_L("\u2103"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_tiStep->Enable(false);
    temp_step_sizer->Add(temp_step_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    temp_step_sizer->Add(m_tiStep, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(temp_step_sizer);

    v_sizer->Add(settings_sizer);
    v_sizer->Add(0, FromDIP(10), 0, wxEXPAND, 5);

    auto dlg_btns = new DialogButtons(this, {"OK"});
    v_sizer->Add(dlg_btns , 0, wxEXPAND);

    dlg_btns->GetOK()->Bind(wxEVT_BUTTON, &Temp_Calibration_Dlg::on_start, this);

    m_rbFilamentType->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(Temp_Calibration_Dlg::on_filament_type_changed), NULL, this);

    //wxGetApp().UpdateDlgDarkUI(this);

    Layout();
    Fit();

    auto validate_text = [this](TextInput* ti){
        unsigned long t = 0;
        if(!ti->GetTextCtrl()->GetValue().ToULong(&t))
            return;
        if(t> 350 || t < 170){
            MessageDialog msg_dlg(nullptr, wxString::Format(L"Supported range: 170%s - 350%s",_L("\u2103"),_L("\u2103")), wxEmptyString, wxICON_WARNING | wxOK);
            msg_dlg.ShowModal();
            if(t > 350)
                t = 350;
            else
                t = 170;
        }
        t = (t / 5) * 5;
        ti->GetTextCtrl()->SetValue(std::to_string(t));
    };

    m_tiStart->GetTextCtrl()->Bind(wxEVT_KILL_FOCUS, [&](wxFocusEvent &e) {
        validate_text(this->m_tiStart);
        e.Skip();
        });

    m_tiEnd->GetTextCtrl()->Bind(wxEVT_KILL_FOCUS, [&](wxFocusEvent &e) {
        validate_text(this->m_tiEnd);
        e.Skip();
        });

    
}

Temp_Calibration_Dlg::~Temp_Calibration_Dlg() {
    // Disconnect Events
    m_rbFilamentType->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(Temp_Calibration_Dlg::on_filament_type_changed), NULL, this);
}

void Temp_Calibration_Dlg::on_start(wxCommandEvent& event) {
    bool read_long = false;
    unsigned long start=0,end=0;
    read_long = m_tiStart->GetTextCtrl()->GetValue().ToULong(&start);
    read_long = read_long && m_tiEnd->GetTextCtrl()->GetValue().ToULong(&end);

    if (!read_long || start > 350 || end < 170  || end > (start - 5)) {
        MessageDialog msg_dlg(nullptr, _L("Please input valid values:\nStart temp: <= 350\nEnd temp: >= 170\nStart temp > End temp + 5)"), wxEmptyString, wxICON_WARNING | wxOK);
        msg_dlg.ShowModal();
        return;
    }
    m_params.start = start;
    m_params.end = end;
    m_params.mode =CalibMode::Calib_Temp_Tower;
    m_plater->calib_temp(m_params);
    EndModal(wxID_OK);

}

void Temp_Calibration_Dlg::on_filament_type_changed(wxCommandEvent& event) {
    int selection = event.GetSelection();
    unsigned long start,end;
    switch(selection)
    {
        case tABS_ASA:
            start = 270;
            end = 230;
            break;
        case tPETG:
            start = 250;
            end = 230;
            break;
	case tPCTG:
            start = 280;
            end = 240;
            break;
        case tTPU:
            start = 240;
            end = 210;
            break;
        case tPA_CF:
            start = 320;
            end = 280;
            break;
        case tPET_CF:
            start = 320;
            end = 280;
            break;
        case tPLA:
        case tCustom:
            start = 230;
            end = 190;
            break;
    }
    
    m_tiEnd->GetTextCtrl()->SetValue(std::to_string(end));
    m_tiStart->GetTextCtrl()->SetValue(std::to_string(start));
    event.Skip();
}

void Temp_Calibration_Dlg::on_dpi_changed(const wxRect& suggested_rect) {
    this->Refresh();
    Fit();

}


// MaxVolumetricSpeed_Test_Dlg
//

MaxVolumetricSpeed_Test_Dlg::MaxVolumetricSpeed_Test_Dlg(wxWindow* parent, wxWindowID id, Plater* plater)
    : DPIDialog(parent, id, _L("Max volumetric speed test"), wxDefaultPosition, parent->FromDIP(wxSize(-1, 280)), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), m_plater(plater)
{
    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(v_sizer);

    // Settings
    //
    wxString start_vol_str = _L("Start volumetric speed: ");
    wxString end_vol_str = _L("End volumetric speed: ");
    wxString vol_step_str = _L("step: ");
    auto text_size = wxWindow::GetTextExtent(start_vol_str);
    text_size.IncTo(wxWindow::GetTextExtent(end_vol_str));
    text_size.IncTo(wxWindow::GetTextExtent(vol_step_str));
    text_size.x = text_size.x * 1.5;
    wxStaticBoxSizer* settings_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _L("Settings"));

    wxString input_str = _L("mm³/s");
    auto input_text_size = wxWindow::GetTextExtent(input_str);

    auto st_size = FromDIP(wxSize(text_size.x, -1));
    auto ti_size = FromDIP(wxSize(input_text_size.x + 90, -1));
    // start vol
    auto start_vol_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto start_vol_text = new wxStaticText(this, wxID_ANY, start_vol_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiStart = new TextInput(this, std::to_string(5), _L("mm³/s"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

    start_vol_sizer->Add(start_vol_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    start_vol_sizer->Add(m_tiStart, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(start_vol_sizer);

    // end vol
    auto end_vol_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto end_vol_text = new wxStaticText(this, wxID_ANY, end_vol_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiEnd = new TextInput(this, std::to_string(20), _L("mm³/s"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    end_vol_sizer->Add(end_vol_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    end_vol_sizer->Add(m_tiEnd, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(end_vol_sizer);

    // vol step
    auto vol_step_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto vol_step_text = new wxStaticText(this, wxID_ANY, vol_step_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiStep = new TextInput(this, wxString::FromDouble(0.5), _L("mm³/s"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    vol_step_sizer->Add(vol_step_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    vol_step_sizer->Add(m_tiStep, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(vol_step_sizer);

    v_sizer->Add(settings_sizer);
    v_sizer->Add(0, FromDIP(10), 0, wxEXPAND, 5);

    auto dlg_btns = new DialogButtons(this, {"OK"});
    v_sizer->Add(dlg_btns , 0, wxEXPAND);

    dlg_btns->GetOK()->Bind(wxEVT_BUTTON, &MaxVolumetricSpeed_Test_Dlg::on_start, this);

    //wxGetApp().UpdateDlgDarkUI(this);

    Layout();
    Fit();
}

MaxVolumetricSpeed_Test_Dlg::~MaxVolumetricSpeed_Test_Dlg() {
    // Disconnect Events
}

void MaxVolumetricSpeed_Test_Dlg::on_start(wxCommandEvent& event) {
    bool read_double = false;
    read_double = m_tiStart->GetTextCtrl()->GetValue().ToDouble(&m_params.start);
    read_double = read_double && m_tiEnd->GetTextCtrl()->GetValue().ToDouble(&m_params.end);
    read_double = read_double && m_tiStep->GetTextCtrl()->GetValue().ToDouble(&m_params.step);

    if (!read_double || m_params.start <= 0 || m_params.step <= 0 || m_params.end < (m_params.start + m_params.step)) {
        MessageDialog msg_dlg(nullptr, _L("Please input valid values:\nstart > 0 \nstep >= 0\nend > start + step)"), wxEmptyString, wxICON_WARNING | wxOK);
        msg_dlg.ShowModal();
        return;
    }

    m_params.mode = CalibMode::Calib_Vol_speed_Tower;
    m_plater->calib_max_vol_speed(m_params);
    EndModal(wxID_OK);

}

void MaxVolumetricSpeed_Test_Dlg::on_dpi_changed(const wxRect& suggested_rect) {
    this->Refresh();
    Fit();

}


// VFA_Test_Dlg
//

VFA_Test_Dlg::VFA_Test_Dlg(wxWindow* parent, wxWindowID id, Plater* plater)
    : DPIDialog(parent, id, _L("VFA test"), wxDefaultPosition, parent->FromDIP(wxSize(-1, 280)), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_plater(plater)
{
    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(v_sizer);

    // Settings
    //
    wxString start_str = _L("Start speed: ");
    wxString end_vol_str = _L("End speed: ");
    wxString vol_step_str = _L("step: ");
    auto text_size = wxWindow::GetTextExtent(start_str);
    text_size.IncTo(wxWindow::GetTextExtent(end_vol_str));
    text_size.IncTo(wxWindow::GetTextExtent(vol_step_str));
    text_size.x = text_size.x * 1.5;
    wxStaticBoxSizer* settings_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _L("Settings"));

    wxString input_str = _L("mm/s");
    auto input_text_size = wxWindow::GetTextExtent(input_str);

    auto st_size = FromDIP(wxSize(text_size.x, -1));
    auto ti_size = FromDIP(wxSize(input_text_size.x + 90, -1));
    // start vol
    auto start_vol_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto start_vol_text = new wxStaticText(this, wxID_ANY, start_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiStart = new TextInput(this, std::to_string(40), _L("mm/s"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

    start_vol_sizer->Add(start_vol_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    start_vol_sizer->Add(m_tiStart, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(start_vol_sizer);

    // end vol
    auto end_vol_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto end_vol_text = new wxStaticText(this, wxID_ANY, end_vol_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiEnd = new TextInput(this, std::to_string(200), _L("mm/s"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    end_vol_sizer->Add(end_vol_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    end_vol_sizer->Add(m_tiEnd, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(end_vol_sizer);

    // vol step
    auto vol_step_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto vol_step_text = new wxStaticText(this, wxID_ANY, vol_step_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiStep = new TextInput(this, wxString::FromDouble(10), _L("mm/s"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    vol_step_sizer->Add(vol_step_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    vol_step_sizer->Add(m_tiStep, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(vol_step_sizer);

    v_sizer->Add(settings_sizer);
    v_sizer->Add(0, FromDIP(10), 0, wxEXPAND, 5);

    auto dlg_btns = new DialogButtons(this, {"OK"});
    v_sizer->Add(dlg_btns , 0, wxEXPAND);

    dlg_btns->GetOK()->Bind(wxEVT_BUTTON, &VFA_Test_Dlg::on_start, this);

    // wxGetApp().UpdateDlgDarkUI(this);

    Layout();
    Fit();
}

VFA_Test_Dlg::~VFA_Test_Dlg()
{
    // Disconnect Events
}

void VFA_Test_Dlg::on_start(wxCommandEvent& event)
{
    bool read_double = false;
    read_double = m_tiStart->GetTextCtrl()->GetValue().ToDouble(&m_params.start);
    read_double = read_double && m_tiEnd->GetTextCtrl()->GetValue().ToDouble(&m_params.end);
    read_double = read_double && m_tiStep->GetTextCtrl()->GetValue().ToDouble(&m_params.step);

    if (!read_double || m_params.start <= 10 || m_params.step <= 0 || m_params.end < (m_params.start + m_params.step)) {
        MessageDialog msg_dlg(nullptr, _L("Please input valid values:\nstart > 10 \nstep >= 0\nend > start + step)"), wxEmptyString, wxICON_WARNING | wxOK);
        msg_dlg.ShowModal();
        return;
    }

    m_params.mode = CalibMode::Calib_VFA_Tower;
    m_plater->calib_VFA(m_params);
    EndModal(wxID_OK);
}

void VFA_Test_Dlg::on_dpi_changed(const wxRect& suggested_rect)
{
    this->Refresh();
    Fit();
}



// Retraction_Test_Dlg
//

Retraction_Test_Dlg::Retraction_Test_Dlg(wxWindow* parent, wxWindowID id, Plater* plater)
    : DPIDialog(parent, id, _L("Retraction test"), wxDefaultPosition, parent->FromDIP(wxSize(-1, 280)), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), m_plater(plater)
{
    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(v_sizer);

    // Settings
    //
    wxString start_length_str = _L("Start retraction length: ");
    wxString end_length_str = _L("End retraction length: ");
    wxString length_step_str = _L("step: ");
    auto text_size = wxWindow::GetTextExtent(start_length_str);
    text_size.IncTo(wxWindow::GetTextExtent(end_length_str));
    text_size.IncTo(wxWindow::GetTextExtent(length_step_str));
    text_size.x = text_size.x * 1.5;
    wxStaticBoxSizer* settings_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _L("Settings"));

    wxString input_text_str = _L("mm/mm");
    auto input_text_size = wxWindow::GetTextExtent(input_text_str);

    auto st_size = FromDIP(wxSize(text_size.x, -1));
    auto ti_size = FromDIP(wxSize(input_text_size.x + 90, -1));
    // start length
    auto start_length_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto start_length_text = new wxStaticText(this, wxID_ANY, start_length_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiStart = new TextInput(this, std::to_string(0), _L("mm"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

    start_length_sizer->Add(start_length_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    start_length_sizer->Add(m_tiStart, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(start_length_sizer);

    // end length
    auto end_length_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto end_length_text = new wxStaticText(this, wxID_ANY, end_length_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiEnd = new TextInput(this, std::to_string(2), _L("mm"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    end_length_sizer->Add(end_length_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    end_length_sizer->Add(m_tiEnd, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(end_length_sizer);

    // length step
    auto length_step_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto length_step_text = new wxStaticText(this, wxID_ANY, length_step_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiStep = new TextInput(this, wxString::FromDouble(0.1), _L("mm/mm"), "", wxDefaultPosition, ti_size, wxTE_RIGHT);
    m_tiStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    length_step_sizer->Add(length_step_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    length_step_sizer->Add(m_tiStep, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(length_step_sizer);

    v_sizer->Add(settings_sizer);
    v_sizer->Add(0, FromDIP(10), 0, wxEXPAND, 5);

    auto dlg_btns = new DialogButtons(this, {"OK"});
    v_sizer->Add(dlg_btns , 0, wxEXPAND);

    dlg_btns->GetOK()->Bind(wxEVT_BUTTON, &Retraction_Test_Dlg::on_start, this);

    //wxGetApp().UpdateDlgDarkUI(this);

    Layout();
    Fit();
}

Retraction_Test_Dlg::~Retraction_Test_Dlg() {
    // Disconnect Events
}

void Retraction_Test_Dlg::on_start(wxCommandEvent& event) {
    bool read_double = false;
    read_double = m_tiStart->GetTextCtrl()->GetValue().ToDouble(&m_params.start);
    read_double = read_double && m_tiEnd->GetTextCtrl()->GetValue().ToDouble(&m_params.end);
    read_double = read_double && m_tiStep->GetTextCtrl()->GetValue().ToDouble(&m_params.step);

    if (!read_double || m_params.start < 0 || m_params.step <= 0 || m_params.end < (m_params.start + m_params.step)) {
        MessageDialog msg_dlg(nullptr, _L("Please input valid values:\nstart > 0 \nstep >= 0\nend > start + step)"), wxEmptyString, wxICON_WARNING | wxOK);
        msg_dlg.ShowModal();
        return;
    }

    m_params.mode = CalibMode::Calib_Retraction_tower;
    m_plater->calib_retraction(m_params);
    EndModal(wxID_OK);

}

void Retraction_Test_Dlg::on_dpi_changed(const wxRect& suggested_rect) {
    this->Refresh();
    Fit();

}

// Input_Shaping_Freq_Test_Dlg
//

Input_Shaping_Freq_Test_Dlg::Input_Shaping_Freq_Test_Dlg(wxWindow* parent, wxWindowID id, Plater* plater)
    : DPIDialog(parent, id, _L("Input shaping Frequency test"), wxDefaultPosition, parent->FromDIP(wxSize(-1, 280)), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), m_plater(plater)
{
    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(v_sizer);

    // Model selection
    wxString m_rbModelChoices[] = { _L("Ringing Tower"), _L("Fast Tower") };
    int m_rbModelNChoices = sizeof(m_rbModelChoices) / sizeof(wxString);
    m_rbModel = new wxRadioBox(this, wxID_ANY, _L("Test model"), wxDefaultPosition, wxDefaultSize, m_rbModelNChoices, m_rbModelChoices, 1, wxRA_SPECIFY_ROWS);
    m_rbModel->SetSelection(0);
    v_sizer->Add(m_rbModel, 0, wxALL | wxEXPAND, 5);

    // Settings
    //
    wxString start_x_str = _L("Start X: ");
    wxString end_x_str = _L("End X: ");
    wxString start_y_str = _L("Start Y: ");
    wxString end_y_str = _L("End Y: ");
    auto text_size = wxWindow::GetTextExtent(start_x_str);
    text_size.IncTo(wxWindow::GetTextExtent(end_x_str));
    text_size.IncTo(wxWindow::GetTextExtent(start_y_str));
    text_size.IncTo(wxWindow::GetTextExtent(end_y_str));
    text_size.x = text_size.x * 1.5;
    wxStaticBoxSizer* settings_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _L("Frequency settings"));

    auto st_size = FromDIP(wxSize(text_size.x, -1));
    auto ti_size = FromDIP(wxSize(90, -1));

    // X axis frequencies
    auto x_freq_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto start_x_text = new wxStaticText(this, wxID_ANY, start_x_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiFreqStartX = new TextInput(this, std::to_string(15), _L("HZ"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiFreqStartX->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    auto end_x_text = new wxStaticText(this, wxID_ANY, end_x_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiFreqEndX = new TextInput(this, std::to_string(110), _L("HZ"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiFreqEndX->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    
    x_freq_sizer->Add(start_x_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    x_freq_sizer->Add(m_tiFreqStartX, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    x_freq_sizer->Add(end_x_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    x_freq_sizer->Add(m_tiFreqEndX, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(x_freq_sizer);

    // Y axis frequencies
    auto y_freq_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto start_y_text = new wxStaticText(this, wxID_ANY, start_y_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiFreqStartY = new TextInput(this, std::to_string(15), _L("HZ"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiFreqStartY->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    auto end_y_text = new wxStaticText(this, wxID_ANY, end_y_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiFreqEndY = new TextInput(this, std::to_string(110), _L("HZ"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiFreqEndY->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    
    y_freq_sizer->Add(start_y_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    y_freq_sizer->Add(m_tiFreqStartY, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    y_freq_sizer->Add(end_y_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    y_freq_sizer->Add(m_tiFreqEndY, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(y_freq_sizer);

    // Damping Factor
    wxString damping_factor_str = _L("Damp: ");
    auto damping_factor_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto damping_factor_text = new wxStaticText(this, wxID_ANY, damping_factor_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiDampingFactor = new TextInput(this, wxString::Format("%.3f", 0.15), "", "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiDampingFactor->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    
    damping_factor_sizer->Add(damping_factor_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    damping_factor_sizer->Add(m_tiDampingFactor, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(damping_factor_sizer);
    
    // Add a note explaining that 0 means use default value
    auto note_text = new wxStaticText(this, wxID_ANY, _L("Recommended: Set Damp to 0.\nThis will use the printer's default or the last saved value."), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    note_text->SetForegroundColour(wxColour(128, 128, 128));
    settings_sizer->Add(note_text, 0, wxALL, 5);

    v_sizer->Add(settings_sizer);
    v_sizer->Add(0, FromDIP(10), 0, wxEXPAND, 5);

    auto dlg_btns = new DialogButtons(this, {"OK"});
    v_sizer->Add(dlg_btns , 0, wxEXPAND);

    dlg_btns->GetOK()->Bind(wxEVT_BUTTON, &Input_Shaping_Freq_Test_Dlg::on_start, this);

    //wxGetApp().UpdateDlgDarkUI(this);//FIXME: dark mode background color

    Layout();
    Fit();
}

Input_Shaping_Freq_Test_Dlg::~Input_Shaping_Freq_Test_Dlg() {
    // Disconnect Events
}

void Input_Shaping_Freq_Test_Dlg::on_start(wxCommandEvent& event) {
    bool read_double = false;
    read_double = m_tiFreqStartX->GetTextCtrl()->GetValue().ToDouble(&m_params.freqStartX);
    read_double = read_double && m_tiFreqEndX->GetTextCtrl()->GetValue().ToDouble(&m_params.freqEndX);
    read_double = read_double && m_tiFreqStartY->GetTextCtrl()->GetValue().ToDouble(&m_params.freqStartY);
    read_double = read_double && m_tiFreqEndY->GetTextCtrl()->GetValue().ToDouble(&m_params.freqEndY);
    read_double = read_double && m_tiDampingFactor->GetTextCtrl()->GetValue().ToDouble(&m_params.start);
    
    if (!read_double ||
        m_params.freqStartX < 0 || m_params.freqEndX > 500 ||
        m_params.freqStartY < 0 || m_params.freqEndX > 500 ||
        m_params.freqStartX >= m_params.freqEndX ||
        m_params.freqStartY >= m_params.freqEndY) {
        MessageDialog msg_dlg(nullptr, _L("Please input valid values\n(0 < FreqStart < FreqEnd < 500"), wxEmptyString, wxICON_WARNING | wxOK);
        msg_dlg.ShowModal();
        return;
    }
    
    if (m_params.start < 0 || m_params.start >= 1) {
        MessageDialog msg_dlg(nullptr, _L("Please input a valid damping factor (0 < Damping/zeta factor <= 1)"), wxEmptyString, wxICON_WARNING | wxOK);
        msg_dlg.ShowModal();
        return;
    }

    m_params.mode = CalibMode::Calib_Input_shaping_freq;
    
    // Set model type based on selection
    m_params.test_model = m_rbModel->GetSelection() == 0 ? 0 : 1; // 0 = Ringing Tower, 1 = Fast Tower
    
    m_plater->calib_input_shaping_freq(m_params);
    EndModal(wxID_OK);
}

void Input_Shaping_Freq_Test_Dlg::on_dpi_changed(const wxRect& suggested_rect) {
    this->Refresh();
    Fit();
}

// Input_Shaping_Damp_Test_Dlg
//

Input_Shaping_Damp_Test_Dlg::Input_Shaping_Damp_Test_Dlg(wxWindow* parent, wxWindowID id, Plater* plater)
    : DPIDialog(parent, id, _L("Input shaping Damp test"), wxDefaultPosition, parent->FromDIP(wxSize(-1, 280)), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), m_plater(plater)
{
    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(v_sizer);

    // Model selection
    wxString m_rbModelChoices[] = { _L("Ringing Tower"), _L("Fast Tower") };
    int m_rbModelNChoices = sizeof(m_rbModelChoices) / sizeof(wxString);
    m_rbModel = new wxRadioBox(this, wxID_ANY, _L("Test model"), wxDefaultPosition, wxDefaultSize, m_rbModelNChoices, m_rbModelChoices, 1, wxRA_SPECIFY_ROWS);
    m_rbModel->SetSelection(0);
    v_sizer->Add(m_rbModel, 0, wxALL | wxEXPAND, 5);

   // Settings
    //
    wxString freq_x_str = _L("Freq X: ");
    wxString freq_y_str = _L("Freq Y: ");
    wxString damp_start_str = _L("Start damp: ");
    wxString damp_end_str = _L("End damp: ");
    auto text_size = wxWindow::GetTextExtent(freq_x_str);
    text_size.IncTo(wxWindow::GetTextExtent(freq_y_str));
    text_size.IncTo(wxWindow::GetTextExtent(damp_start_str));
    text_size.IncTo(wxWindow::GetTextExtent(damp_end_str));
    text_size.x = text_size.x * 1.5;
    wxStaticBoxSizer* settings_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _L("Frequency settings"));

    auto st_size = FromDIP(wxSize(text_size.x, -1));
    auto ti_size = FromDIP(wxSize(90, -1));

    auto freq_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto freq_x_text = new wxStaticText(this, wxID_ANY, freq_x_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiFreqX = new TextInput(this, std::to_string(30), _L("HZ"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiFreqX->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    auto freq_y_text = new wxStaticText(this, wxID_ANY, freq_y_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiFreqY = new TextInput(this, std::to_string(30), _L("HZ"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiFreqY->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    
    freq_sizer->Add(freq_x_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    freq_sizer->Add(m_tiFreqX, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    freq_sizer->Add(freq_y_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    freq_sizer->Add(m_tiFreqY, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(freq_sizer);
    
    // Damping Factor Start and End
    auto damp_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto damp_start_text = new wxStaticText(this, wxID_ANY, damp_start_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiDampingFactorStart = new TextInput(this, wxString::Format("%.3f", 0.00), "", "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiDampingFactorStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    auto damp_end_text = new wxStaticText(this, wxID_ANY, damp_end_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiDampingFactorEnd = new TextInput(this, wxString::Format("%.3f", 0.40), "", "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiDampingFactorEnd->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    
    damp_sizer->Add(damp_start_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    damp_sizer->Add(m_tiDampingFactorStart, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    damp_sizer->Add(damp_end_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    damp_sizer->Add(m_tiDampingFactorEnd, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(damp_sizer);
   
    // Add a note to explain users to use their previously calculated frequency
    auto note_text = new wxStaticText(this, wxID_ANY, _L("Note: Use previously calculated frequencies."), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    note_text->SetForegroundColour(wxColour(128, 128, 128));
    settings_sizer->Add(note_text, 0, wxALL, 5);

    note_text->SetForegroundColour(wxColour(128, 128, 128));
    settings_sizer->Add(note_text, 0, wxALL, 5);

    v_sizer->Add(settings_sizer);
    v_sizer->Add(0, FromDIP(10), 0, wxEXPAND, 5);

    auto dlg_btns = new DialogButtons(this, {"OK"});
    v_sizer->Add(dlg_btns , 0, wxEXPAND);

    dlg_btns->GetOK()->Bind(wxEVT_BUTTON, &Input_Shaping_Damp_Test_Dlg::on_start, this);

    //wxGetApp().UpdateDlgDarkUI(this);//FIXME: dark mode background color

    Layout();
    Fit();
}

Input_Shaping_Damp_Test_Dlg::~Input_Shaping_Damp_Test_Dlg() {
    // Disconnect Events
}

void Input_Shaping_Damp_Test_Dlg::on_start(wxCommandEvent& event) {
    bool read_double = false;
    read_double = m_tiFreqX->GetTextCtrl()->GetValue().ToDouble(&m_params.freqStartX);
    read_double = read_double && m_tiFreqY->GetTextCtrl()->GetValue().ToDouble(&m_params.freqStartY);
    read_double = read_double && m_tiDampingFactorStart->GetTextCtrl()->GetValue().ToDouble(&m_params.start);
    read_double = read_double && m_tiDampingFactorEnd->GetTextCtrl()->GetValue().ToDouble(&m_params.end);

        
    if (!read_double ||
        m_params.freqStartX < 0 || m_params.freqStartX > 500 ||
        m_params.freqStartY < 0 || m_params.freqStartY > 500 ) {
        MessageDialog msg_dlg(nullptr, _L("Please input valid values\n(0 < Freq < 500"), wxEmptyString, wxICON_WARNING | wxOK);
        msg_dlg.ShowModal();
        return;
    }

    if (m_params.start < 0 || m_params.end > 1
        || m_params.start >= m_params.end) {
        MessageDialog msg_dlg(nullptr, _L("Please input a valid damping factor (0 <= DampingStart < DampingEnd <= 1)"), wxEmptyString, wxICON_WARNING | wxOK);
        msg_dlg.ShowModal();
        return;
    }

    m_params.mode = CalibMode::Calib_Input_shaping_damp;
    
    // Set model type based on selection
    m_params.test_model = m_rbModel->GetSelection() == 0 ? 0 : 1; // 0 = Ringing Tower, 1 = Fast Tower
    
    m_plater->calib_input_shaping_damp(m_params);
    EndModal(wxID_OK);
}

void Input_Shaping_Damp_Test_Dlg::on_dpi_changed(const wxRect& suggested_rect) {
    this->Refresh();
    Fit();
}

// Junction_Deviation_Test_Dlg
//

Junction_Deviation_Test_Dlg::Junction_Deviation_Test_Dlg(wxWindow* parent, wxWindowID id, Plater* plater)
    : DPIDialog(parent, id, _L("Junction Deviation test"), wxDefaultPosition, parent->FromDIP(wxSize(-1, 280)), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), m_plater(plater)
{
    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(v_sizer);

    // Model selection
    wxString m_rbModelChoices[] = { _L("Ringing Tower"), _L("Fast Tower") };
    int m_rbModelNChoices = sizeof(m_rbModelChoices) / sizeof(wxString);
    m_rbModel = new wxRadioBox(this, wxID_ANY, _L("Test model"), wxDefaultPosition, wxDefaultSize, m_rbModelNChoices, m_rbModelChoices, 1, wxRA_SPECIFY_ROWS);
    m_rbModel->SetSelection(1);
    v_sizer->Add(m_rbModel, 0, wxALL | wxEXPAND, 5);

    // Settings
    wxString start_jd_str = _L("Start junction deviation: ");
    wxString end_jd_str = _L("End junction deviation: ");
    auto text_size = wxWindow::GetTextExtent(start_jd_str);
    text_size.IncTo(wxWindow::GetTextExtent(end_jd_str));
    text_size.x = text_size.x * 1.5;
    wxStaticBoxSizer* settings_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _L("Junction Deviation settings"));

    auto st_size = FromDIP(wxSize(text_size.x, -1));
    auto ti_size = FromDIP(wxSize(90, -1));

    // Start junction deviation
    auto start_jd_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto start_jd_text = new wxStaticText(this, wxID_ANY, start_jd_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiJDStart = new TextInput(this, wxString::Format("%.3f", 0.000), _L("mm"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiJDStart->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    start_jd_sizer->Add(start_jd_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    start_jd_sizer->Add(m_tiJDStart, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(start_jd_sizer);

    // End junction deviation
    auto end_jd_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto end_jd_text = new wxStaticText(this, wxID_ANY, end_jd_str, wxDefaultPosition, st_size, wxALIGN_LEFT);
    m_tiJDEnd = new TextInput(this, wxString::Format("%.3f", 0.250), _L("mm"), "", wxDefaultPosition, ti_size, wxTE_CENTRE);
    m_tiJDEnd->GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    end_jd_sizer->Add(end_jd_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    end_jd_sizer->Add(m_tiJDEnd, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settings_sizer->Add(end_jd_sizer);

    // Add note about junction deviation
    auto note_text = new wxStaticText(this, wxID_ANY, _L("Note: Lower values = sharper corners but slower speeds"), 
                                    wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    note_text->SetForegroundColour(wxColour(128, 128, 128));
    settings_sizer->Add(note_text, 0, wxALL, 5);

    v_sizer->Add(settings_sizer);
    v_sizer->Add(0, FromDIP(10), 0, wxEXPAND, 5);

    auto dlg_btns = new DialogButtons(this, {"OK"});
    v_sizer->Add(dlg_btns , 0, wxEXPAND);

    dlg_btns->GetOK()->Bind(wxEVT_BUTTON, &Junction_Deviation_Test_Dlg::on_start, this);

    Layout();
    Fit();
}

Junction_Deviation_Test_Dlg::~Junction_Deviation_Test_Dlg() {
    // Disconnect Events
}

void Junction_Deviation_Test_Dlg::on_start(wxCommandEvent& event) {
    bool read_double = false;
    read_double = m_tiJDStart->GetTextCtrl()->GetValue().ToDouble(&m_params.start);
    read_double = read_double && m_tiJDEnd->GetTextCtrl()->GetValue().ToDouble(&m_params.end);

    if (!read_double || m_params.start < 0 || m_params.end >= 1 || m_params.start >= m_params.end) {
        MessageDialog msg_dlg(nullptr, _L("Please input valid values\n(0 <= Junction Deviation < 1)"), wxEmptyString, wxICON_WARNING | wxOK);
        msg_dlg.ShowModal();
        return;
    } else if (m_params.end > 0.3) {
        MessageDialog msg_dlg(nullptr, _L("NOTE: High values may cause Layer shift"), wxEmptyString, wxICON_WARNING | wxOK);
        msg_dlg.ShowModal();
    }

    m_params.mode = CalibMode::Calib_Junction_Deviation;
    
    // Set model type based on selection
    m_params.test_model = m_rbModel->GetSelection() == 0 ? 0 : 1; // 0 = Ringing Tower, 1 = Fast Tower
    
    m_plater->calib_junction_deviation(m_params);
    EndModal(wxID_OK);
}

void Junction_Deviation_Test_Dlg::on_dpi_changed(const wxRect& suggested_rect) {
    this->Refresh();
    Fit();
}

}} // namespace Slic3r::GUI