#pragma once
// arsip.hpp — Semua definisi struct untuk sistem deteksi duplikasi data arsip digital.
// Tidak ada implementasi di sini, hanya type definitions.

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

// ============================================================
// DATA UTAMA
// ============================================================

/**
 * DataArsip — Satu record file arsip dalam sistem.
 */
struct DataArsip {
    std::string id;        // Unik, format "D<angka>"
    std::string namaFile;  // Nama file (key utama)
    long long   ukuran;    // Ukuran dalam bytes
    std::string tanggal;   // Tanggal modifikasi file
    std::string sumber;    // Folder asal file
    std::string konten;    // Isi file (string)
};

/**
 * GrupDuplikat — Kelompok file yang memiliki konten atau metadata identik.
 */
struct GrupDuplikat {
    long long                ukuran;    // Ukuran file dalam grup
    std::vector<std::string> namaFile;  // Nama-nama file dalam grup ini
};

/**
 * HasilDeteksi — Output satu run deteksi duplikasi.
 */
struct HasilDeteksi {
    std::vector<GrupDuplikat> grup;
    double waktuMs;
    size_t estimasiMemoriBytes;
};

// ============================================================
// PERFORMA & EKSPERIMEN
// ============================================================

/**
 * CatatanPerforma — Rekaman satu skenario eksperimen.
 * Mencatat waktu untuk semua operasi pada ketiga struktur data.
 */
struct CatatanPerforma {
    int jumlahData;
    int persenDuplikat;

    // Deteksi by konten (ms)
    double deteksiHashMs;
    double deteksiBstMs;
    double deteksiVectorMs;

    // Deteksi by metadata (ms)
    double deteksiMetaHashMs;
    double deteksiMetaBstMs;
    double deteksiMetaVectorMs;

    // Insert batch (ms)
    double insertHashMs;
    double insertBstMs;
    double insertVectorMs;

    // Search single — ada (ms)
    double searchHashMs;
    double searchBstMs;
    double searchVectorMs;

    // Search single — tidak ada / worst case (ms)
    double searchMissHashMs;
    double searchMissBstMs;
    double searchMissVectorMs;

    // Range search ukuran (ms)
    double rangeSearchBstMs;
    double rangeSearchVectorMs;

    // Delete single (ms)
    double deleteHashMs;
    double deleteBstMs;
    double deleteVectorMs;

    // Estimasi memori (bytes)
    size_t memoriHashBytes;
    size_t memoriBstBytes;
    size_t memoriVectorBytes;

    // Hasil duplikasi
    int jumlahGrupDuplikat;
    int jumlahFileDuplikat;

    int replikasi = 1;  // Run ke-berapa (1, 2, 3)
};

// ============================================================
// INDEKS TERPADU — TIGA STRUKTUR DATA
// ============================================================

/**
 * ArsipIndex — Kontainer utama yang menggabungkan ketiga struktur data.
 *
 * Tidak ada global variable. Semua fungsi menerima ArsipIndex& sebagai parameter.
 *
 * Kompleksitas:
 *   Hash Table  — Insert O(1) avg, Search O(1) avg, Delete O(1) avg
 *   BST/Map     — Insert O(log n), Search O(log n), Delete O(log n)
 *   Vector      — Insert O(1) amortized, Search O(n), Delete O(n)
 */
struct ArsipIndex {
    // Hash Table — O(1) avg
    std::unordered_map<std::string, DataArsip>              hashByNama;
    std::unordered_map<std::string, DataArsip>              hashById;
    std::unordered_map<long long, std::vector<std::string>> hashByUkuran;

    // BST Ordered Map — O(log n)
    std::map<long long,   std::vector<std::string>>         bstByUkuran;
    std::map<std::string, DataArsip>                        bstByNama;

    // Vector — O(n)
    std::vector<DataArsip>                                  files;

    long long   counter = 1;
    std::string folder  = "arsip_digital";
};
