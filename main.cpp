#include <iostream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <memory>
#include <array>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <atomic>

using std::string;
using std::cout;
using std::endl;

std::atomic<bool> running(true);

void executeCommand(const string& command) {
    std::array<char, 128> buffer;
    std::string result;

    // Abre un pipe para leer la salida del comando
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen((command + " 2>&1").c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");

    // Lee la salida del comando
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    cout << result; // Mostrar la salida del comando
}

void inputH() {
    char input;
    while (running) {
        cout << "Presiona 'p' para pausar, 'r' para reanudar y 'q' para salir: ";
        std::cin >> input;

        if (input == 'p') {
            cout << "Pausando grabación..." << endl;
            // Detener la grabación
            system("pkill -f ffmpeg");
        } else if (input == 'r') {
            cout << "Reanudando grabación..." << endl;
            // Comando de FFmpeg para reiniciar la grabación
            string ffmpegCommand = "ffmpeg -f x11grab -video_size 1920x1080 -i :1.0 "
                                   "-f pulse -i default -c:v libx264 -preset ultrafast "
                                   "-pix_fmt yuv420p grabacion.mp4 &";
            // Reiniciar la grabación
            executeCommand(ffmpegCommand);
        } else if (input == 'q') {
            cout << "Saliendo..." << endl;
            running = false; // Detener el hilo de entrada
        }
    }
}

int main() {
    // Determina el sistema operativo
    #if defined(__linux__)
        cout << "Sistema operativo: Linux" << endl;
        setenv("DISPLAY", ":1", 1);
        std::string ffmpegCommand = "ffmpeg -f x11grab -video_size 1920x1080 -i :1.0 "
                                     "-f pulse -i default -c:v libx264 -preset ultrafast "
                                     "-pix_fmt yuv420p grabacion.mp4";
    #elif defined(_WIN32)
        cout << "Sistema operativo: Windows" << endl;
        std::string ffmpegCommand = "ffmpeg -f gdigrab -framerate 30 -i desktop "
                                     "-f dshow -i audio=\"Microphone (Tu Micrófono)\" "
                                     "-c:v libx264 -preset ultrafast -pix_fmt yuv420p grabacion.mp4";
    #elif defined(__APPLE__)
        cout << "Sistema operativo: macOS" << endl;
        std::string ffmpegCommand = "ffmpeg -f avfoundation -framerate 30 -i \"0\" "
                                     "-f avfoundation -i \":0\" "
                                     "-c:v libx264 -preset ultrafast -pix_fmt yuv420p grabacion.mp4";
    #else
        cout << "Sistema operativo no soportado." << endl;
        return 1;
    #endif

    // Ejecutar la grabación en un hilo separado
    std::thread ffmpegThread(executeCommand, ffmpegCommand);

    // Manejar la entrada del usuario
    inputH();

    // Esperar a que el hilo de FFmpeg termine
    if (ffmpegThread.joinable()) {
        ffmpegThread.join();
    }

    return 0;
}
