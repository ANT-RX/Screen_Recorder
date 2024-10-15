cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(ScreenRecorder)

# Especificar el estándar de C++
set(CMAKE_CXX_STANDARD 11)

# Buscar las librerías de wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)

# Verificar que wxWidgets fue encontrado
if(wxWidgets_FOUND)
    # Incluir directorios
    include(${wxWidgets_USE_FILE})
else()
    message(FATAL_ERROR "wxWidgets no fue encontrado!")
endif()

# Añadir el ejecutable
add_executable(SRRIS main.cpp)

# Linkear las bibliotecas necesarias (como wxWidgets y pthread para threading en Linux)
target_link_libraries(SRRIS ${wxWidgets_LIBRARIES} pthread)
