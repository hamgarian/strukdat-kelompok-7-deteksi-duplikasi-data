#pragma once
// utils.hpp — Fungsi utilitas bebas state.
// Tidak bergantung pada ArsipIndex atau modul lain.

#include <cstddef>
#include <string>

/** Gabungkan path folder dan nama file dengan separator yang benar (cross-platform). */
std::string gabung_path(const std::string& folder, const std::string& nama_file);

/** Format ukuran bytes menjadi string yang mudah dibaca: "1.50 KB", "2.00 MB", dst. */
std::string format_bytes(size_t bytes);

/** Baca tanggal modifikasi file dari stat(). Kembalikan "Unknown" jika gagal. */
std::string cek_tanggal(const std::string& file_path);

/** Buat bar ASCII untuk grafik console. nilai/nilai_maks dikali lebar karakter. */
std::string bar(double nilai, double nilai_maks, int lebar = 45);

/**
 * Buat folder (cross-platform).
 * Windows: _mkdir | POSIX: mkdir(0755)
 * Tidak ada error jika folder sudah ada.
 */
void buat_folder(const std::string& path);
