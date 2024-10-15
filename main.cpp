#include <wx/wx.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <memory>
#include <array>
#include <stdexcept>
#include <thread>
#include <atomic>

using std::string;
using std::cout;
using std::endl;

std::atomic<bool> running(true);

void executeCommand(const string& command) {
    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen((command + " 2>&1").c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    cout << result;
}

// Clase principal que maneja la interfaz gráfica
class ScreenRecorderApp : public wxFrame {
public:
    ScreenRecorderApp(const wxString& title);

    void OnPause(wxCommandEvent& event);
    void OnResume(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);

private:
    std::thread ffmpegThread;
    wxButton* pauseButton;
    wxButton* resumeButton;
    wxButton* quitButton;

    string ffmpegCommand;

    wxDECLARE_EVENT_TABLE();
};

// Eventos para los botones
enum {
    ID_Pause = 1,
    ID_Resume,
    ID_Quit
};

wxBEGIN_EVENT_TABLE(ScreenRecorderApp, wxFrame)
    EVT_BUTTON(ID_Pause, ScreenRecorderApp::OnPause)
    EVT_BUTTON(ID_Resume, ScreenRecorderApp::OnResume)
    EVT_BUTTON(ID_Quit, ScreenRecorderApp::OnQuit)
wxEND_EVENT_TABLE()

// Implementación de la interfaz gráfica
ScreenRecorderApp::ScreenRecorderApp(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(300, 200)) {
    
    wxPanel* panel = new wxPanel(this, -1);

    pauseButton = new wxButton(panel, ID_Pause, wxT("Pausar"), wxPoint(10, 10));
    resumeButton = new wxButton(panel, ID_Resume, wxT("Reanudar"), wxPoint(10, 50));
    quitButton = new wxButton(panel, ID_Quit, wxT("Salir"), wxPoint(10, 90));

    // Iniciar el comando de FFmpeg según el sistema operativo
    #if defined(__linux__)
        ffmpegCommand = "ffmpeg -f x11grab -video_size 1920x1080 -i :1.0 "
                        "-f pulse -i default -c:v libx264 -preset ultrafast "
                        "-pix_fmt yuv420p grabacion.mp4";
    #elif defined(_WIN32)
        ffmpegCommand = "ffmpeg -f gdigrab -framerate 30 -i desktop "
                        "-f dshow -i audio=\"Microphone (Tu Micrófono)\" "
                        "-c:v libx264 -preset ultrafast -pix_fmt yuv420p grabacion.mp4";
    #elif defined(__APPLE__)
        ffmpegCommand = "ffmpeg -f avfoundation -framerate 30 -i \"0\" "
                        "-f avfoundation -i \":0\" "
                        "-c:v libx264 -preset ultrafast -pix_fmt yuv420p grabacion.mp4";
    #else
        cout << "Sistema operativo no soportado." << endl;
        return;
    #endif

    // Iniciar grabación
    ffmpegThread = std::thread(executeCommand, ffmpegCommand);
}

void ScreenRecorderApp::OnPause(wxCommandEvent& event) {
    cout << "Pausando grabación..." << endl;
    system("pkill -f ffmpeg");
}

void ScreenRecorderApp::OnResume(wxCommandEvent& event) {
    cout << "Reanudando grabación..." << endl;
    ffmpegThread = std::thread(executeCommand, ffmpegCommand + " &");
}

void ScreenRecorderApp::OnQuit(wxCommandEvent& event) {
    cout << "Saliendo..." << endl;
    running = false;
    Close(true);
}

// Clase principal
class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    ScreenRecorderApp* frame = new ScreenRecorderApp(wxT("Screen Recorder"));
    frame->Show(true);
    return true;
}
