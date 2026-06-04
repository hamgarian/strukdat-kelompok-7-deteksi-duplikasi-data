#pragma once
// detection.hpp — Deteksi duplikasi data: 3 by konten + 1 by metadata (3 metode).

#include "arsip.hpp"

// ============================================================
// BY KONTEN
// ============================================================

/**
 * Deteksi duplikat by konten — Hash Table.
 * Algoritma: group by ukuran (hashByUkuran) → compare konten dalam grup.
 * Kompleksitas: O(n) avg.
 */
HasilDeteksi deteksi_hash(ArsipIndex& idx, bool tampil_detail);

/**
 * Deteksi duplikat by konten — BST (Ordered Map).
 * Algoritma: in-order traverse bstByUkuran → compare konten dalam grup.
 * Kompleksitas: O(n log n).
 */
HasilDeteksi deteksi_bst(ArsipIndex& idx, bool tampil_detail);

/**
 * Deteksi duplikat by konten — Vector Linear Search.
 * Algoritma: nested loop — bandingkan konten setiap pasang file.
 * Kompleksitas: O(n²) worst case.
 */
HasilDeteksi deteksi_vector(ArsipIndex& idx, bool tampil_detail);

// ============================================================
// BY METADATA
// ============================================================

/**
 * Deteksi duplikat by metadata — membandingkan tiga metode sekaligus.
 * Metadata = nama file + ukuran (tanpa baca konten).
 * @param tampil_detail  Jika true, cetak daftar grup duplikat
 *
 * Keluaran: tiga HasilDeteksi (hash, bst, vector) lewat parameter out.
 */
void deteksi_metadata(ArsipIndex& idx, bool tampil_detail,
                      HasilDeteksi& out_hash,
                      HasilDeteksi& out_bst,
                      HasilDeteksi& out_vector);

// ============================================================
// HELPER
// ============================================================

/** Tampilkan daftar grup duplikat (max batas grup pertama). */
void cetak_grup_duplikat(const std::vector<GrupDuplikat>& grup, int batas = 20);

/** Hitung jumlah total file yang masuk ke dalam grup duplikat. */
int hitung_file_duplikat(const std::vector<GrupDuplikat>& grup);
