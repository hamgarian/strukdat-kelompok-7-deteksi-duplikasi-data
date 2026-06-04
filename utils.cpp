// utils.cpp — Implementasi fungsi utilitas.

#include "utils.hpp"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

#ifdef _WIN32
#   include <direct.h>
#else
#   include <sys/types.h>
#endif

std::string gabung_path(const std::string& folder, const std::string& nama_file) {
    if (folder.empty()) return nama_file;
    char akhir = folder[folder.size() - 1];
    if (akhir == '/' || akhir == '\\') return folder + nama_file;
    return folder + "/" + nama_file;
}

std::string format_bytes(size_t bytes) {
    const char* satuan[] = {"B", "KB", "MB", "GB"};
    double nilai = static_cast<double>(bytes);
    int idx = 0;
    while (nilai >= 1024.0 && idx < 3) { nilai /= 1024.0; ++idx; }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(idx == 0 ? 0 : 2) << nilai << " " << satuan[idx];
    return ss.str();
}

std::string cek_tanggal(const std::string& file_path) {
    struct stat info;
    if (stat(file_path.c_str(), &info) == 0) {
        time_t t = info.st_mtime;
        struct tm* lt = localtime(&t);
        char buf[80];
        strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", lt);
        return std::string(buf);
    }
    return "Unknown";
}

std::string bar(double nilai, double nilai_maks, int lebar) {
    if (nilai_maks <= 0.0) return "";
    int panjang = static_cast<int>((nilai / nilai_maks) * lebar);
    panjang = std::max(1, std::min(lebar, panjang));
    return std::string(panjang, '#');
}

void buat_folder(const std::string& path) {
#ifdef _WIN32
    _mkdir(path.c_str());
#else
    mkdir(path.c_str(), 0755);
#endif
}
