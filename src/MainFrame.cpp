#pragma once
#include "MainFrame.h"
#include "Exec.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <array>


//Main window
MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "Simple Command Runner")
{
    std::cout << "Simple Command Runner by matyalatte" << std::endl;
    
    //get gui definition
    LoadDefinition();
    LoadConfig();

    //make menu bar
    wxMenuBar* menuBar = new wxMenuBar;
    wxMenu* menuFile = new wxMenu;
    
    if (definition["gui"].size() > 1) {
        for (int i = 0; i < definition["gui"].size(); i++) {
            menuFile->Append(wxID_HIGHEST + i + 1, definition["gui"][i]["name"]);
            menuFile->Bind(wxEVT_MENU, &MainFrame::UpdateFrame, this, wxID_HIGHEST + i + 1);
        }
    }
    menuFile->Append(wxID_EXIT, "Quit");
    menuBar->Append(menuFile, "Menu");

    //put help urls to menu bar
    if (hasKey(definition, "help")) {
        wxMenu* menuHelp = new wxMenu;

        for (int i = 0; i < definition["help"].size(); i++) {
            menuHelp->Append(wxID_HIGHEST + i + 1 + definition["gui"].size(), definition["help"][i]["label"]);
            menuHelp->Bind(wxEVT_MENU, &MainFrame::OpenURL, this, wxID_HIGHEST + i + 1 + definition["gui"].size());
        }
        menuBar->Append(menuHelp, "Help");
    }
    SetMenuBar(menuBar);

    //set close event
    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
    Bind(wxEVT_MENU, [this](wxCommandEvent&) { Close(true); }, wxID_EXIT);
    
    //put components
    mainPanel = new wxPanel(this);
    components = std::vector<Component>();
    int y = UpdatePanel(mainPanel);

    //run button
    wxButton* button = new wxButton(mainPanel, wxID_EXECUTE, sub_definition["button"], wxPoint(158, y), wxSize(75, 25));
    Connect(wxID_EXECUTE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::RunCommand));
    
    mainPanel->Show();

    Layout();
    Centre();
    SetSize(wxSize(405, y+105));
    SetWindowStyleFlag(wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER & ~wxMAXIMIZE_BOX);
}

//get default definition of gui
nlohmann::json default_definition(std::string msg) {
    std::cout << msg << std::endl;
    nlohmann::json def =
        {
#ifdef _WIN32
            {"command", {"dir"} },
            {"button", "run 'dir'"},
#else
            {"command", {"ls"} },
            {"button", "run 'ls'"},
#endif
            {"components",{}}
        };
    return def;
}

/*
bool hasKey(nlohmann::json json, std::string key) {
    auto subjectIdIter = json.find(key);
    return subjectIdIter != json.end();
}
*/
//read gui_definition.json
void MainFrame::LoadDefinition() {
    std::ifstream istream("gui_definition.json");

    if (!istream) {
        sub_definition = default_definition("[LoadDefinition] Fialed to read gui_definition.json");
        definition = { { "gui", {sub_definition}} };
        return;
    }

    //read json file
    istream >> definition;
    istream.close();

    //check format
    if (hasKey(definition, "gui")) {
        sub_definition = definition["gui"][0];
    }
    else {
        sub_definition = default_definition("[LoadDefinition] Json format error ('gui' not found)");
        return;
    }
    std::vector<std::string> keys = { "name", "button", "command", "components" };
    for (std::string key : keys) {
        if (!hasKey(sub_definition, key)) {
            sub_definition = default_definition("[LoadDefinition] Json format error ('" + key + "' not found)");
            return;
        }
    }
    std::cout << "[LoadDefinition] Loaded gui_definition.json" << std::endl;
}

void MainFrame::UpdateConfig() {
    for (Component c: components){
        config[c.GetLabel()] = c.GetConfig();
    }
}
void MainFrame::LoadConfig() {
    std::ifstream istream("gui_config.json");
    if (!istream) {
        std::cout << "[LoadConfig] Fialed to load gui_config.json" << std::endl;
        config = {};
    }
    else {
        std::cout << "[LoadConfig] Loaded gui_config.json" << std::endl;
        istream >> config;
        istream.close();
    }
}

void MainFrame::SaveConfig() {
    UpdateConfig();
    std::ofstream ostream("gui_config.json");

    if (!ostream) {
        std::cout << "[SaveConfig] Fialed to write gui_config.json" << std::endl;
    }
    else {
        std::cout << "[SaveConfig] Saved gui_config.json" << std::endl;
        ostream << std::setw(4) << config << std::endl;
        ostream.close();
    }
}

//run command
void MainFrame::RunCommand(wxCommandEvent& event) {
    //save config
    SaveConfig();

    //make command string
    std::vector<std::string> cmd_ary = sub_definition["command"];
    wxString cmd = "";
    int i = 0;
    for (Component c : components) {
        if (c.HasString()) {
            cmd += cmd_ary[i] + c.GetString();
            i += 1;
        }
    }

    while (cmd_ary.size() > i) {
        cmd += cmd_ary[i];
        i += 1;
    }

    //run command
    std::cout << "[RunCommand] command: " << cmd << std::endl;
#ifdef _WIN32
    cmd = "cmd.exe /c " + cmd;
#endif
    std::vector<std::string> msg = exec(cmd);

    if (msg[0] == "__null__") {
        std::cout << "[RunCommand] Execution failed. " << std::endl;
        return;
    }

    //show result
    wxMessageDialog* dialog;
    if (msg[1] != "") {//if error
        dialog = new wxMessageDialog(this, msg[1], "Error", wxICON_ERROR | wxOK | wxCENTRE);
    }
    else {//if success
        if (hasKey(sub_definition, "show_last_line") && sub_definition["show_last_line"]!=0 && msg[0]!="") {
            dialog = new wxMessageDialog(this, msg[0], "Success");
        }
        else {
            dialog = new wxMessageDialog(this, "Success!", "Success");
        }
    }
    dialog->ShowModal();
    dialog->Destroy();
}

MainFrame::~MainFrame(){}

void MainFrame::OpenURL(wxCommandEvent& event) {
    std::string url = definition["help"][event.GetId() - 1 - wxID_HIGHEST - definition["gui"].size()]["url"];
    std::cout << "[OpenURL] " << url << std::endl;
    bool res = wxLaunchDefaultBrowser(url);
}

void MainFrame::UpdateFrame(wxCommandEvent& event)
{
    sub_definition = definition["gui"][event.GetId() - 1 - wxID_HIGHEST];
    UpdateConfig();
    
    wxPanel* newPanel = new wxPanel(this);
    int y = UpdatePanel(newPanel);
    wxButton* button = new wxButton(newPanel, wxID_EXECUTE, sub_definition["button"], wxPoint(158, y), wxSize(75, 25));
    newPanel->Show();
    wxPanel* unused = mainPanel;
    mainPanel = newPanel;
    unused->Destroy();

    Layout();
    SetSize(wxSize(405, y + 105));
    Refresh();
}

//put components
int MainFrame::UpdatePanel(wxPanel* panel)
{
    std::string str = "Simple Command Runner";
    if (hasKey(sub_definition, "name")) {
        str = sub_definition["name"];
    }
    std::cout << "[UpdatePanel] " << str.c_str() << std::endl;
    if (hasKey(sub_definition, "label")) {
        str = sub_definition["label"];
    }
    SetLabel(str);

    //file picker
    int y = 10;
    if (sub_definition["components"].is_null()) {
        sub_definition["components"] = std::vector<nlohmann::json>();
        return y;
    }
    std::vector<nlohmann::json> comp = sub_definition["components"];
    components.clear();
    components.shrink_to_fit();
    Component* newComp = nullptr;

    //put components
    for (nlohmann::json c : comp) {
        newComp = Component::PutComponent(panel, c, y);
        if (newComp != nullptr) {
            y += newComp->GetHeight();
            if (hasKey(config, newComp->GetLabel())) {
                newComp->SetConfig(config[newComp->GetLabel()]);
            }
            components.push_back(*newComp);
        }
    }
    return y;
}

void MainFrame::OnClose(wxCloseEvent& event)
{
    Destroy();
}