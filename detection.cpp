// detection.cpp — Implementasi deteksi duplikasi data.

#include "detection.hpp"
#include "storage.hpp"

#include <chrono>
#include <iostream>
#include <unordered_map>

// ============================================================
// HELPER
// ============================================================
void cetak_grup_duplikat(const std::vector<GrupDuplikat>& grup, int batas) {
    if (grup.empty()) { std::cout << "Tidak ada duplikasi yang ditemukan.\n"; return; }
    int tampil = std::min(static_cast<int>(grup.size()), batas);
    for (int i = 0; i < tampil; i++) {
        std::cout << "-> DUPLIKAT (Ukuran: " << grup[i].ukuran << " bytes) | File: ";
        for (const std::string& n : grup[i].namaFile) std::cout << n << " ";
        std::cout << "\n";
    }
    if (static_cast<int>(grup.size()) > batas)
        std::cout << "... " << (grup.size() - batas) << " grup lain disembunyikan.\n";
}

int hitung_file_duplikat(const std::vector<GrupDuplikat>& grup) {
    int total = 0;
    for (const GrupDuplikat& g : grup) total += static_cast<int>(g.namaFile.size());
    return total;
}

// ============================================================
// BY KONTEN — HASH TABLE   O(n) avg
// ============================================================
HasilDeteksi deteksi_hash(ArsipIndex& idx, bool tampil_detail) {
    auto t0 = std::chrono::high_resolution_clock::now();
    std::vector<GrupDuplikat> hasil;

    for (const auto& ku : idx.hashByUkuran) {
        if (ku.second.size() <= 1) continue;
        std::unordered_map<std::string, std::vector<std::string>> konten_map;
        for (const std::string& nama : ku.second) {
            auto it = idx.hashByNama.find(nama);
            if (it != idx.hashByNama.end())
                konten_map[it->second.konten].push_back(nama);
        }
        for (const auto& kv : konten_map)
            if (kv.second.size() > 1) hasil.push_back({ku.first, kv.second});
    }

    double ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();
    if (tampil_detail) cetak_grup_duplikat(hasil);
    return {hasil, ms, estimasi_memori_hash(idx)};
}

// ============================================================
// BY KONTEN — BST (in-order traversal)   O(n log n)
// ============================================================
HasilDeteksi deteksi_bst(ArsipIndex& idx, bool tampil_detail) {
    auto t0 = std::chrono::high_resolution_clock::now();
    std::vector<GrupDuplikat> hasil;

    // bstByUkuran sudah terurut — in-order traversal otomatis
    for (const auto& ku : idx.bstByUkuran) {
        if (ku.second.size() <= 1) continue;
        std::unordered_map<std::string, std::vector<std::string>> konten_map;
        for (const std::string& nama : ku.second) {
            auto it = idx.bstByNama.find(nama);
            if (it != idx.bstByNama.end())
                konten_map[it->second.konten].push_back(nama);
        }
        for (const auto& kv : konten_map)
            if (kv.second.size() > 1) hasil.push_back({ku.first, kv.second});
    }

    double ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();
    if (tampil_detail) cetak_grup_duplikat(hasil);
    return {hasil, ms, estimasi_memori_bst(idx)};
}

// ============================================================
// BY KONTEN — VECTOR NESTED LOOP   O(n²)
// ============================================================
HasilDeteksi deteksi_vector(ArsipIndex& idx, bool tampil_detail) {
    struct GrupLinear {
        long long ukuran; std::string konten; std::vector<std::string> namaFile;
    };

    auto t0 = std::chrono::high_resolution_clock::now();
    std::vector<GrupLinear> grup_linear;

    int iter_count = 0;
    for (const DataArsip& d : idx.files) {
        bool ditemukan = false;
        for (GrupLinear& g : grup_linear) {
            if (g.ukuran == d.ukuran && g.konten == d.konten) {
                g.namaFile.push_back(d.namaFile); ditemukan = true; break;
            }
            iter_count++;
            // Timeout check setiap 50.000 komparasi
            if (iter_count > 50000) {
                iter_count = 0;
                double elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t0).count();
                if (elapsed > 10.0) { // Timeout 10 detik
                    std::cout << "\n[!] Vector O(n^2) TIMEOUT (>10 detik). Operasi dihentikan karena skala data Ekstrem.\n";
                    return {std::vector<GrupDuplikat>{}, std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count(), estimasi_memori_vector(idx)};
                }
            }
        }
        if (!ditemukan) grup_linear.push_back({d.ukuran, d.konten, {d.namaFile}});
    }

    std::vector<GrupDuplikat> hasil;
    for (const GrupLinear& g : grup_linear)
        if (g.namaFile.size() > 1) hasil.push_back({g.ukuran, g.namaFile});

    double ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();
    if (tampil_detail) cetak_grup_duplikat(hasil);
    return {hasil, ms, estimasi_memori_vector(idx)};
}

// ============================================================
// BY METADATA — 3 metode sekaligus
// ============================================================
void deteksi_metadata(ArsipIndex& idx, bool tampil_detail,
                      HasilDeteksi& out_hash,
                      HasilDeteksi& out_bst,
                      HasilDeteksi& out_vector) {
    // --- Hash: group by (namaFile + "|" + ukuran) ---
    {
        auto t0 = std::chrono::high_resolution_clock::now();
        std::unordered_map<std::string, std::vector<std::string>> meta_map;
        for (const DataArsip& d : idx.files)
            meta_map[d.namaFile + "|" + std::to_string(d.ukuran)].push_back(d.namaFile);

        std::vector<GrupDuplikat> hasil;
        for (const auto& kv : meta_map) {
            if (kv.second.size() > 1) {
                auto it = idx.hashByNama.find(kv.second[0]);
                long long uk = (it != idx.hashByNama.end()) ? it->second.ukuran : 0;
                hasil.push_back({uk, kv.second});
            }
        }
        double ms = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
        if (tampil_detail) cetak_grup_duplikat(hasil);
        out_hash = {hasil, ms, estimasi_memori_hash(idx)};
    }

    // --- BST: in-order traverse, group by (nama+ukuran) dalam tiap node ---
    {
        auto t0 = std::chrono::high_resolution_clock::now();
        std::vector<GrupDuplikat> hasil;

        for (const auto& ku : idx.bstByUkuran) {
            // Dalam satu node ukuran, cek apakah ada nama yang sama
            std::unordered_map<std::string, std::vector<std::string>> nama_map;
            for (const std::string& nama : ku.second)
                nama_map[nama].push_back(nama);
            for (const auto& kv : nama_map)
                if (kv.second.size() > 1) hasil.push_back({ku.first, kv.second});
        }

        double ms = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
        if (tampil_detail) cetak_grup_duplikat(hasil);
        out_bst = {hasil, ms, estimasi_memori_bst(idx)};
    }

    // --- Vector: O(n²) — bandingkan (nama+ukuran) tiap pasang ---
    {
        int n = static_cast<int>(idx.files.size());
        auto t0 = std::chrono::high_resolution_clock::now();
        std::vector<bool> diproses(n, false);
        std::vector<GrupDuplikat> hasil;

        int iter_count = 0;
        bool timeout = false;
        for (int i = 0; i < n && !timeout; i++) {
            if (diproses[i]) continue;
            std::vector<std::string> grup = {idx.files[i].namaFile};
            for (int j = i + 1; j < n; j++) {
                if (!diproses[j]
                    && idx.files[i].namaFile == idx.files[j].namaFile
                    && idx.files[i].ukuran   == idx.files[j].ukuran) {
                    grup.push_back(idx.files[j].namaFile);
                    diproses[j] = true;
                }
                iter_count++;
                if (iter_count > 50000) {
                    iter_count = 0;
                    double elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t0).count();
                    if (elapsed > 10.0) {
                        std::cout << "\n[!] Vector Metadata O(n^2) TIMEOUT (>10 detik). Operasi dihentikan.\n";
                        timeout = true; break;
                    }
                }
            }
            if (grup.size() > 1) {
                hasil.push_back({idx.files[i].ukuran, grup});
                diproses[i] = true;
            }
        }

        double ms = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
        if (tampil_detail) cetak_grup_duplikat(hasil);
        out_vector = {hasil, ms, estimasi_memori_vector(idx)};
    }
}
