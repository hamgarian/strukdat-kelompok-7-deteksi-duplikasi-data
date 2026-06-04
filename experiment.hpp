#pragma once
// experiment.hpp — Benchmark CRUD, eksperimen otomatis, dan pelaporan (CSV + HTML + console).

#include "arsip.hpp"
#include <vector>
#include <string>

// ============================================================
// BENCHMARK CRUD
// ============================================================

/**
 * Ukur waktu insert batch untuk semua data di idx.files ke dalam tiga DS terpisah.
 * Struktur asli idx tidak dimodifikasi — diukur pada salinan internal.
 */
struct HasilInsert { double hashMs; double bstMs; double vectorMs; };
HasilInsert ukur_insert(const std::vector<DataArsip>& sampel);

/**
 * Ukur waktu search single pada ketiga DS (O(1) vs O(log n) vs O(n)).
 * @param target_nama  Nama file yang dicari
 */
struct HasilSearch { double hashMs; double bstMs; double vectorMs; };
HasilSearch ukur_search(const ArsipIndex& idx, const std::string& target_nama);

/**
 * Ukur waktu search untuk nama yang TIDAK ADA (worst-case / miss).
 * Nama yang digunakan adalah "__TIDAK_ADA__.txt".
 */
HasilSearch ukur_search_miss(const ArsipIndex& idx);

/**
 * Ukur waktu range search by ukuran (BST lower_bound vs Vector linear).
 * Hash tidak mendukung range query.
 */
struct HasilRange { double bstMs; double vectorMs; };
HasilRange ukur_range(const ArsipIndex& idx, long long min_b, long long max_b);

/**
 * Ukur waktu delete single dari salinan ArsipIndex (agar data asli tidak rusak).
 * @param idx_copy     Salinan ArsipIndex (di-pass by value)
 * @param target_nama  Nama file yang dihapus
 */
struct HasilDelete { double hashMs; double bstMs; double vectorMs; };
HasilDelete ukur_delete(ArsipIndex idx_copy, const std::string& target_nama);

// ============================================================
// EKSPERIMEN & PERBANDINGAN
// ============================================================

/**
 * Bandingkan ketiga metode pada data yang sedang di-load di idx.
 * Tampilkan ringkasan + simpan CSV dan HTML ke folder output/.
 */
void jalankan_perbandingan(ArsipIndex& idx, std::vector<CatatanPerforma>& riwayat,
                           int persen_duplikat = 10);

/**
 * Eksperimen otomatis:
 * Loop {100, 500, 1000, 5000, 10000} × {5, 15, 35}% × replikasi 3x.
 * Setiap iterasi: populate → load → ukur semua operasi → catat rata-rata.
 * Total: 15 skenario × 3 replikasi = 45 run.
 */
void jalankan_eksperimen_otomatis(ArsipIndex& idx, std::vector<CatatanPerforma>& riwayat);

// ============================================================
// PELAPORAN
// ============================================================

/** Tampilkan ringkasan HasilDeteksi satu baris ke console. */
void tampilkan_ringkasan(const std::string& label, const HasilDeteksi& hasil);

/**
 * Tampilkan grafik bar ASCII ke console.
 * Menampilkan Hash, BST, dan Vector berjejer untuk setiap skenario.
 */
void tampilkan_grafik_console(const std::vector<CatatanPerforma>& data);

/**
 * Tampilkan statistik lengkap:
 * total file, % unik, % duplikat, distribusi ukuran (min/max/rata/median),
 * estimasi memori ketiga DS.
 */
void tampilkan_statistik_lengkap(ArsipIndex& idx);

/**
 * Simpan semua CatatanPerforma ke output/performance_report.csv.
 * Mencakup semua kolom: insert/search/delete/deteksi/metadata/memori × 3 DS.
 */
void simpan_csv(const std::vector<CatatanPerforma>& data);

/**
 * Generate grafik HTML ke output/grafik_performa.html.
 * Warna: Hash=#0f766e, BST=#7c3aed, Vector=#c2410c.
 */
void simpan_html(const std::vector<CatatanPerforma>& data);
