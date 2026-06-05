#pragma once
// storage.hpp — Operasi disk: load data ke memori, tulis file, generate dummy data.

#include "arsip.hpp"
#include <string>

/**
 * Load semua file dari idx.folder ke dalam ArsipIndex (ketiga DS).
 * Menggunakan std::filesystem::directory_iterator (C++17).
 * Reset indeks sebelum load.
 */
void load_data_ke_memori(ArsipIndex& idx);

/**
 * Tulis konten ke file di disk.
 * @param folder     Folder tujuan (dibuat jika belum ada)
 * @param nama_file  Nama file
 * @param isi        Konten yang ditulis
 * @return true jika berhasil
 */
bool tulis_file(const std::string& folder, const std::string& nama_file,
                const std::string& isi);

/**
 * Tambah satu DataArsip ke ketiga struktur data dalam ArsipIndex sekaligus.
 * Fungsi ini memastikan semua DS selalu sinkron.
 */
void tambah_ke_indeks(ArsipIndex& idx, const DataArsip& data);

/**
 * Hapus entry dengan namaFile dari ketiga DS.
 * Tidak ada operasi jika nama tidak ditemukan.
 */
void hapus_dari_indeks(ArsipIndex& idx, const std::string& nama_file);

/**
 * Generate file dummy untuk eksperimen.
 * @param idx             ArsipIndex target
 * @param folder          Folder tempat file dibuat
 * @param jumlah          Jumlah file yang di-generate
 * @param persen_duplikat 0–100: persen file yang merupakan salinan file lain
 */
void populate_data(ArsipIndex& idx, const std::string& folder,
                   int jumlah, int persen_duplikat = 10);

/**
 * Generate data dummy skala besar langsung ke memori (RAM).
 */
void populate_data_in_memory(ArsipIndex& idx, int jumlah, int persen_duplikat = 15);

// --- Operasi CRUD ---

/** Cari DataArsip by ID menggunakan Hash (O(1) avg). Kembalikan nullptr jika tidak ada. */
const DataArsip* cari_by_id(const ArsipIndex& idx, const std::string& id);

/** Cari DataArsip by nama menggunakan Hash (O(1) avg). */
const DataArsip* cari_by_nama_hash(const ArsipIndex& idx, const std::string& nama);

/** Cari DataArsip by nama menggunakan BST (O(log n)). */
const DataArsip* cari_by_nama_bst(const ArsipIndex& idx, const std::string& nama);

/** Cari DataArsip by nama menggunakan Vector linear scan (O(n)). */
const DataArsip* cari_by_nama_vector(const ArsipIndex& idx, const std::string& nama);

/**
 * Range query by ukuran via BST lower_bound/upper_bound.
 * O(log n + k), k = jumlah hasil.
 */
std::vector<DataArsip> cari_by_range_ukuran(const ArsipIndex& idx,
                                             long long min_bytes, long long max_bytes);

/** Range query by ukuran via Vector linear scan (O(n)). */
std::vector<DataArsip> cari_by_range_ukuran_vector(const ArsipIndex& idx,
                                                    long long min_bytes, long long max_bytes);

// --- Estimasi Memori ---
/** Estimasi memori Hash Table (pendekatan manual, bukan heap profiler). */
size_t estimasi_memori_hash(const ArsipIndex& idx);

/** Estimasi memori BST (setiap node ≈ 4 pointer overhead). */
size_t estimasi_memori_bst(const ArsipIndex& idx);

/** Estimasi memori Vector (sizeof + capacity * sizeof(DataArsip) + string data). */
size_t estimasi_memori_vector(const ArsipIndex& idx);
