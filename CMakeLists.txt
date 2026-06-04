cmake_minimum_required(VERSION 3.10)
project(DeteksiDuplikasiData LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(deteksi_duplikasi
    main.cpp
    utils.cpp
    storage.cpp
    detection.cpp
    experiment.cpp
)
