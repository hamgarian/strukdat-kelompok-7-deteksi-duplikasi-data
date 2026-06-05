// experiment.cpp — Implementasi benchmark CRUD, eksperimen otomatis, dan pelaporan.

#include "experiment.hpp"
#include "detection.hpp"
#include "storage.hpp"
#include "utils.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>



// ============================================================
// BENCHMARK CRUD
// ============================================================

HasilInsert ukur_insert(const std::vector<DataArsip>& sampel) {
    HasilInsert h{};

    // Hash Table insert
    {
        std::unordered_map<std::string, DataArsip> tmpNama;
        std::unordered_map<std::string, DataArsip> tmpId;
        std::unordered_map<long long, std::vector<std::string>> tmpUkuran;
        auto t0 = std::chrono::high_resolution_clock::now();
        for (const DataArsip& d : sampel) {
            tmpNama[d.namaFile] = d;
            tmpId[d.id] = d;
            tmpUkuran[d.ukuran].push_back(d.namaFile);
        }
        h.hashMs = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
    }

    // BST insert
    {
        std::map<std::string, DataArsip> tmpNama;
        std::map<long long, std::vector<std::string>> tmpUkuran;
        auto t0 = std::chrono::high_resolution_clock::now();
        for (const DataArsip& d : sampel) {
            tmpNama[d.namaFile] = d;
            tmpUkuran[d.ukuran].push_back(d.namaFile);
        }
        h.bstMs = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
    }

    // Vector insert
    {
        std::vector<DataArsip> tmpVec;
        auto t0 = std::chrono::high_resolution_clock::now();
        for (const DataArsip& d : sampel) {
            tmpVec.push_back(d);
        }
        h.vectorMs = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
    }

    return h;
}

HasilSearch ukur_search(const ArsipIndex& idx, const std::string& target_nama) {
    HasilSearch h{};
    auto t0 = std::chrono::high_resolution_clock::now();
    cari_by_nama_hash(idx, target_nama);
    h.hashMs = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();

    t0 = std::chrono::high_resolution_clock::now();
    cari_by_nama_bst(idx, target_nama);
    h.bstMs = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();

    t0 = std::chrono::high_resolution_clock::now();
    cari_by_nama_vector(idx, target_nama);
    h.vectorMs = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();

    return h;
}

HasilSearch ukur_search_miss(const ArsipIndex& idx) {
    return ukur_search(idx, "__TIDAK_ADA__.txt");
}

HasilRange ukur_range(const ArsipIndex& idx, long long min_b, long long max_b) {
    HasilRange h{};
    auto t0 = std::chrono::high_resolution_clock::now();
    cari_by_range_ukuran(idx, min_b, max_b);
    h.bstMs = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();

    t0 = std::chrono::high_resolution_clock::now();
    cari_by_range_ukuran_vector(idx, min_b, max_b);
    h.vectorMs = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();

    return h;
}

HasilDelete ukur_delete(ArsipIndex idx_copy, const std::string& target_nama) {
    HasilDelete h{};

    // Measure hash delete on a copy
    {
        ArsipIndex tmp = idx_copy;
        auto t0 = std::chrono::high_resolution_clock::now();
        tmp.hashByNama.erase(target_nama);
        // Also erase from hashById and hashByUkuran for realism
        h.hashMs = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
    }

    // Measure BST delete
    {
        ArsipIndex tmp = idx_copy;
        auto t0 = std::chrono::high_resolution_clock::now();
        tmp.bstByNama.erase(target_nama);
        h.bstMs = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
    }

    // Measure vector delete
    {
        ArsipIndex tmp = idx_copy;
        auto t0 = std::chrono::high_resolution_clock::now();
        tmp.files.erase(
            std::remove_if(tmp.files.begin(), tmp.files.end(),
                [&](const DataArsip& d) { return d.namaFile == target_nama; }),
            tmp.files.end());
        h.vectorMs = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
    }

    return h;
}

// ============================================================
// TAMPILAN & PELAPORAN
// ============================================================

void tampilkan_ringkasan(const std::string& label, const HasilDeteksi& hasil) {
    std::cout << "------------------------------------------\n";
    std::cout << "[!] METODE              : " << label << "\n";
    std::cout << "[!] WAKTU EKSEKUSI      : " << std::fixed << std::setprecision(4)
              << hasil.waktuMs << " ms\n";
    std::cout << "[!] ESTIMASI MEMORI     : " << format_bytes(hasil.estimasiMemoriBytes) << "\n";
    std::cout << "[!] GRUP DUPLIKAT       : " << hasil.grup.size() << "\n";
    std::cout << "[!] FILE DALAM DUPLIKAT : " << hitung_file_duplikat(hasil.grup) << "\n";
    std::cout << "------------------------------------------\n";
}

void tampilkan_grafik_console(const std::vector<CatatanPerforma>& data) {
    if (data.empty()) {
        std::cout << "[!] Belum ada data eksperimen.\n";
        return;
    }

    double waktu_maks = 0.0;
    size_t memori_maks = 0;
    for (const CatatanPerforma& c : data) {
        waktu_maks = std::max(waktu_maks, std::max({c.deteksiHashMs, c.deteksiBstMs, c.deteksiVectorMs}));
        memori_maks = std::max(memori_maks, std::max({c.memoriHashBytes, c.memoriBstBytes, c.memoriVectorBytes}));
    }

    std::cout << "\n=== Grafik Waktu Deteksi Konten (ms) ===\n";
    for (const CatatanPerforma& c : data) {
        std::cout << "[" << std::setw(5) << c.jumlahData << "/" << std::setw(2) << c.persenDuplikat
                  << "%] Hash   | " << bar(c.deteksiHashMs, waktu_maks)
                  << " " << std::fixed << std::setprecision(3) << c.deteksiHashMs << " ms\n";
        std::cout << "[" << std::setw(5) << c.jumlahData << "/" << std::setw(2) << c.persenDuplikat
                  << "%] BST    | " << bar(c.deteksiBstMs, waktu_maks)
                  << " " << std::fixed << std::setprecision(3) << c.deteksiBstMs << " ms\n";
        std::cout << "[" << std::setw(5) << c.jumlahData << "/" << std::setw(2) << c.persenDuplikat
                  << "%] Vector | " << bar(c.deteksiVectorMs, waktu_maks)
                  << " " << std::fixed << std::setprecision(3) << c.deteksiVectorMs << " ms\n";
    }

    std::cout << "\n=== Grafik Estimasi Memori ===\n";
    for (const CatatanPerforma& c : data) {
        double dm = static_cast<double>(memori_maks);
        std::cout << "[" << std::setw(5) << c.jumlahData << "] Hash   | "
                  << bar(static_cast<double>(c.memoriHashBytes), dm) << " "
                  << format_bytes(c.memoriHashBytes) << "\n";
        std::cout << "[" << std::setw(5) << c.jumlahData << "] BST    | "
                  << bar(static_cast<double>(c.memoriBstBytes), dm) << " "
                  << format_bytes(c.memoriBstBytes) << "\n";
        std::cout << "[" << std::setw(5) << c.jumlahData << "] Vector | "
                  << bar(static_cast<double>(c.memoriVectorBytes), dm) << " "
                  << format_bytes(c.memoriVectorBytes) << "\n";
    }
}

void tampilkan_statistik_lengkap(ArsipIndex& idx) {
    std::cout << "\n=== STATISTIK DATA ARSIP ===\n";
    int total = static_cast<int>(idx.files.size());
    std::cout << "Jumlah file              : " << total << "\n";

    if (total == 0) {
        std::cout << "[!] Tidak ada data.\n";
        return;
    }

    long long total_ukuran = 0;
    for (const DataArsip& d : idx.files) total_ukuran += d.ukuran;

    std::cout << "Total ukuran file        : " << format_bytes(static_cast<size_t>(total_ukuran)) << "\n";
    std::cout << "Estimasi memori Hash     : " << format_bytes(estimasi_memori_hash(idx)) << "\n";
    std::cout << "Estimasi memori BST      : " << format_bytes(estimasi_memori_bst(idx)) << "\n";
    std::cout << "Estimasi memori Vector   : " << format_bytes(estimasi_memori_vector(idx)) << "\n";

    auto minmax = std::minmax_element(idx.files.begin(), idx.files.end(),
        [](const DataArsip& a, const DataArsip& b) { return a.ukuran < b.ukuran; });

    double rata_rata = static_cast<double>(total_ukuran) / total;
    std::cout << "Rata-rata ukuran file    : " << format_bytes(static_cast<size_t>(rata_rata)) << "\n";
    std::cout << "File terkecil            : " << minmax.first->namaFile
              << " (" << format_bytes(static_cast<size_t>(minmax.first->ukuran)) << ")\n";
    std::cout << "File terbesar            : " << minmax.second->namaFile
              << " (" << format_bytes(static_cast<size_t>(minmax.second->ukuran)) << ")\n";

    // Median
    std::vector<long long> ukurans;
    for (const DataArsip& d : idx.files) ukurans.push_back(d.ukuran);
    std::sort(ukurans.begin(), ukurans.end());
    double median = (total % 2 == 0)
        ? (ukurans[total/2 - 1] + ukurans[total/2]) / 2.0
        : static_cast<double>(ukurans[total/2]);
    std::cout << "Median ukuran file       : " << format_bytes(static_cast<size_t>(median)) << "\n";

    HasilDeteksi h = deteksi_hash(idx, false);
    int dup = hitung_file_duplikat(h.grup);
    int unik = total - dup;
    std::cout << "Grup duplikat            : " << h.grup.size() << "\n";
    std::cout << "File duplikat            : " << dup << " (" << std::fixed << std::setprecision(1)
              << (100.0 * dup / total) << "%)\n";
    std::cout << "File unik                : " << unik << " (" << std::fixed << std::setprecision(1)
              << (100.0 * unik / total) << "%)\n";
}

// ============================================================
// PERBANDINGAN (data saat ini)
// ============================================================
void jalankan_perbandingan(ArsipIndex& idx, std::vector<CatatanPerforma>& riwayat,
                           int persen_duplikat) {
    if (idx.files.empty()) {
        std::cout << "[!] Memori kosong. Generate atau insert data terlebih dahulu.\n";
        return;
    }

    std::cout << "\n=== PERBANDINGAN 3 STRUKTUR DATA ===\n";
    HasilDeteksi rHash   = deteksi_hash(idx, false);
    HasilDeteksi rBst    = deteksi_bst(idx, false);
    HasilDeteksi rVector = deteksi_vector(idx, false);

    tampilkan_ringkasan("Hash Table", rHash);
    tampilkan_ringkasan("BST (Ordered Map)", rBst);
    tampilkan_ringkasan("Vector Linear Search", rVector);

    // CRUD benchmark
    std::string target = idx.files[0].namaFile;
    HasilInsert  hi = ukur_insert(idx.files);
    HasilSearch  hs = ukur_search(idx, target);
    HasilSearch  hm = ukur_search_miss(idx);
    HasilRange   hr = ukur_range(idx, 0, 500000);
    HasilDelete  hd = ukur_delete(idx, target);

    // Metadata
    HasilDeteksi mHash, mBst, mVec;
    deteksi_metadata(idx, false, mHash, mBst, mVec);

    CatatanPerforma c{};
    c.jumlahData       = static_cast<int>(idx.files.size());
    c.persenDuplikat   = persen_duplikat;
    c.deteksiHashMs    = rHash.waktuMs;
    c.deteksiBstMs     = rBst.waktuMs;
    c.deteksiVectorMs  = rVector.waktuMs;
    c.deteksiMetaHashMs   = mHash.waktuMs;
    c.deteksiMetaBstMs    = mBst.waktuMs;
    c.deteksiMetaVectorMs = mVec.waktuMs;
    c.insertHashMs     = hi.hashMs;
    c.insertBstMs      = hi.bstMs;
    c.insertVectorMs   = hi.vectorMs;
    c.searchHashMs     = hs.hashMs;
    c.searchBstMs      = hs.bstMs;
    c.searchVectorMs   = hs.vectorMs;
    c.searchMissHashMs = hm.hashMs;
    c.searchMissBstMs  = hm.bstMs;
    c.searchMissVectorMs = hm.vectorMs;
    c.rangeSearchBstMs    = hr.bstMs;
    c.rangeSearchVectorMs = hr.vectorMs;
    c.deleteHashMs     = hd.hashMs;
    c.deleteBstMs      = hd.bstMs;
    c.deleteVectorMs   = hd.vectorMs;
    c.memoriHashBytes  = rHash.estimasiMemoriBytes;
    c.memoriBstBytes   = rBst.estimasiMemoriBytes;
    c.memoriVectorBytes = rVector.estimasiMemoriBytes;
    c.jumlahGrupDuplikat = static_cast<int>(rHash.grup.size());
    c.jumlahFileDuplikat = hitung_file_duplikat(rHash.grup);

    riwayat.push_back(c);
    tampilkan_grafik_console(riwayat);
    simpan_csv(riwayat);
    simpan_html(riwayat);
    std::cout << "[System] Laporan disimpan ke output/performance_report.csv dan output/grafik_performa.html.\n";
}

// ============================================================
// EKSPERIMEN OTOMATIS — 45 run
// ============================================================
void jalankan_eksperimen_otomatis(ArsipIndex& idx, std::vector<CatatanPerforma>& riwayat) {
    // 1. Pengembangan, 2. Observasi, 3. Ekstrem/Stress Test
    std::vector<int> ukuran_ds  = {1000, 5000, 10000};
    std::vector<int> persen_dup = {10, 15, 20};
    const int REP = 3;

    riwayat.clear();
    std::cout << "\n=== EKSPERIMEN OTOMATIS (" << ukuran_ds.size() << " dataset x "
              << persen_dup.size() << " skenario x " <<REP << " replikasi) ===\n";

    for (int n : ukuran_ds) {
        for (int p : persen_dup) {
            std::cout << "\n--- Dataset " << n << " file, " << p << "% duplikat ---\n";
            for (int r = 1; r <= REP; r++) {
                std::cout << "  Replikasi " << r << "/" << REP << "...\n";
                populate_data(idx, idx.folder, n, p);
                load_data_ke_memori(idx);

                HasilDeteksi rH = deteksi_hash(idx, false);
                HasilDeteksi rB = deteksi_bst(idx, false);
                HasilDeteksi rV = deteksi_vector(idx, false);

                std::string target = idx.files.empty() ? "" : idx.files[0].namaFile;
                HasilInsert  hi = ukur_insert(idx.files);
                HasilSearch  hs = idx.files.empty() ? HasilSearch{} : ukur_search(idx, target);
                HasilSearch  hm = ukur_search_miss(idx);
                HasilRange   hr = ukur_range(idx, 0, 500000);
                HasilDelete  hd = idx.files.empty() ? HasilDelete{} : ukur_delete(idx, target);

                HasilDeteksi mH, mB, mV;
                deteksi_metadata(idx, false, mH, mB, mV);

                CatatanPerforma c{};
                c.jumlahData       = n;
                c.persenDuplikat   = p;
                c.replikasi        = r;
                c.deteksiHashMs    = rH.waktuMs;
                c.deteksiBstMs     = rB.waktuMs;
                c.deteksiVectorMs  = rV.waktuMs;
                c.deteksiMetaHashMs   = mH.waktuMs;
                c.deteksiMetaBstMs    = mB.waktuMs;
                c.deteksiMetaVectorMs = mV.waktuMs;
                c.insertHashMs     = hi.hashMs;
                c.insertBstMs      = hi.bstMs;
                c.insertVectorMs   = hi.vectorMs;
                c.searchHashMs     = hs.hashMs;
                c.searchBstMs      = hs.bstMs;
                c.searchVectorMs   = hs.vectorMs;
                c.searchMissHashMs = hm.hashMs;
                c.searchMissBstMs  = hm.bstMs;
                c.searchMissVectorMs = hm.vectorMs;
                c.rangeSearchBstMs    = hr.bstMs;
                c.rangeSearchVectorMs = hr.vectorMs;
                c.deleteHashMs     = hd.hashMs;
                c.deleteBstMs      = hd.bstMs;
                c.deleteVectorMs   = hd.vectorMs;
                c.memoriHashBytes  = rH.estimasiMemoriBytes;
                c.memoriBstBytes   = rB.estimasiMemoriBytes;
                c.memoriVectorBytes = rV.estimasiMemoriBytes;
                c.jumlahGrupDuplikat = static_cast<int>(rH.grup.size());
                c.jumlahFileDuplikat = hitung_file_duplikat(rH.grup);

                riwayat.push_back(c);

                std::cout << " Hash=" << std::fixed << std::setprecision(2) << rH.waktuMs
                          << "ms BST=" << rB.waktuMs << "ms Vec=" << rV.waktuMs << "ms\n";
            }
        }
    }

    tampilkan_grafik_console(riwayat);
    simpan_csv(riwayat);
    simpan_html(riwayat);
    std::cout << "\n[System] Eksperimen selesai. " << riwayat.size() << " run dicatat.\n";
    std::cout << "[System] Laporan: output/performance_report.csv & output/grafik_performa.html\n";
}

// ============================================================
// SIMPAN CSV
// ============================================================
void simpan_csv(const std::vector<CatatanPerforma>& data) {
    buat_folder("output");
    std::ofstream f("output/performance_report.csv");
    f << "jumlah_data,persen_duplikat,replikasi,"
      << "deteksi_hash_ms,deteksi_bst_ms,deteksi_vector_ms,"
      << "deteksi_meta_hash_ms,deteksi_meta_bst_ms,deteksi_meta_vector_ms,"
      << "insert_hash_ms,insert_bst_ms,insert_vector_ms,"
      << "search_hash_ms,search_bst_ms,search_vector_ms,"
      << "search_miss_hash_ms,search_miss_bst_ms,search_miss_vector_ms,"
      << "range_bst_ms,range_vector_ms,"
      << "delete_hash_ms,delete_bst_ms,delete_vector_ms,"
      << "memori_hash_bytes,memori_bst_bytes,memori_vector_bytes,"
      << "grup_duplikat,file_duplikat\n";

    for (const CatatanPerforma& c : data) {
        f << c.jumlahData << "," << c.persenDuplikat << "," << c.replikasi << ","
          << std::fixed << std::setprecision(6)
          << c.deteksiHashMs << "," << c.deteksiBstMs << "," << c.deteksiVectorMs << ","
          << c.deteksiMetaHashMs << "," << c.deteksiMetaBstMs << "," << c.deteksiMetaVectorMs << ","
          << c.insertHashMs << "," << c.insertBstMs << "," << c.insertVectorMs << ","
          << c.searchHashMs << "," << c.searchBstMs << "," << c.searchVectorMs << ","
          << c.searchMissHashMs << "," << c.searchMissBstMs << "," << c.searchMissVectorMs << ","
          << c.rangeSearchBstMs << "," << c.rangeSearchVectorMs << ","
          << c.deleteHashMs << "," << c.deleteBstMs << "," << c.deleteVectorMs << ","
          << c.memoriHashBytes << "," << c.memoriBstBytes << "," << c.memoriVectorBytes << ","
          << c.jumlahGrupDuplikat << "," << c.jumlahFileDuplikat << "\n";
    }
}

// ============================================================
// SIMPAN HTML
// ============================================================
void simpan_html(const std::vector<CatatanPerforma>& data) {
    if (data.empty()) return;
    buat_folder("output");

    double waktu_maks = 0.0;
    size_t memori_maks = 0;
    for (const CatatanPerforma& c : data) {
        waktu_maks = std::max(waktu_maks, std::max({c.deteksiHashMs, c.deteksiBstMs, c.deteksiVectorMs}));
        memori_maks = std::max(memori_maks, std::max({c.memoriHashBytes, c.memoriBstBytes, c.memoriVectorBytes}));
    }

    std::ofstream html("output/grafik_performa.html");
    if (data.empty()) return;
    
    // Ambil data tes yang terakhir (ukuran paling besar/akhir) untuk ditampilkan di grafik
    const CatatanPerforma& c = data.back();
    
    double memHashMB = c.memoriHashBytes / (1024.0 * 1024.0);
    double memBstMB = c.memoriBstBytes / (1024.0 * 1024.0);
    double memVecMB = c.memoriVectorBytes / (1024.0 * 1024.0);

    html << R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Grafik Performa 3 DS</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body { font-family: Arial, sans-serif; display: flex; flex-direction: column; align-items: center; background-color: #ffffff; padding: 40px; }
        .chart-wrapper { width: 700px; background: #fff; padding: 20px; margin-bottom: 40px; }
    </style>
</head>
<body>
    <div class="chart-wrapper">
        <canvas id="waktuChart"></canvas>
        <div style="text-align: center; margin-top: 20px; font-family: 'Times New Roman', Times, serif; font-size: 16px;">
            Gambar 6.1 Perbandingan Waktu Deteksi Konten (Skala: )" << c.jumlahData << R"( data)
        </div>
    </div>
    
    <div class="chart-wrapper">
        <canvas id="memoriChart"></canvas>
        <div style="text-align: center; margin-top: 20px; font-family: 'Times New Roman', Times, serif; font-size: 16px;">
            Gambar 6.2 Perbandingan Estimasi Memori (Skala: )" << c.jumlahData << R"( data)
        </div>
    </div>

    <script>
        // Chart 1: Waktu Deteksi (ms)
        const ctxWaktu = document.getElementById('waktuChart').getContext('2d');
        new Chart(ctxWaktu, {
            type: 'bar',
            data: {
                labels: ['Hash Table', 'BST', 'Vector'],
                datasets: [{
                    data: [)" << c.deteksiHashMs << ", " << c.deteksiBstMs << ", " << c.deteksiVectorMs << R"(],
                    backgroundColor: '#4285F4',
                    maxBarThickness: 120
                }]
            },
            options: {
                plugins: {
                    title: { 
                        display: true, 
                        text: 'Waktu Deteksi (ms) vs Struktur Data', 
                        font: { size: 24, family: 'Arial', weight: 'normal' }, 
                        align: 'start', 
                        color: '#5f6368',
                        padding: { bottom: 20 }
                    },
                    legend: { display: false }
                },
                scales: {
                    y: { 
                        beginAtZero: true, 
                        grid: { color: '#e0e0e0', drawBorder: false },
                        ticks: { font: { size: 14 } },
                        title: { display: true, text: 'Waktu Deteksi (ms)', font: { size: 14 } }
                    },
                    x: { 
                        grid: { display: false },
                        ticks: { font: { size: 14 } },
                        title: { display: true, text: 'Struktur Data', font: { size: 14 }, padding: { top: 10 } }
                    }
                }
            }
        });

        // Chart 2: Estimasi Memori (MB)
        const ctxMemori = document.getElementById('memoriChart').getContext('2d');
        new Chart(ctxMemori, {
            type: 'bar',
            data: {
                labels: ['Hash Table', 'BST', 'Vector'],
                datasets: [{
                    data: [)" << memHashMB << ", " << memBstMB << ", " << memVecMB << R"(],
                    backgroundColor: '#4285F4',
                    maxBarThickness: 120
                }]
            },
            options: {
                plugins: {
                    title: { 
                        display: true, 
                        text: 'Estimasi Memori vs Struktur Data', 
                        font: { size: 24, family: 'Arial', weight: 'normal' }, 
                        align: 'start', 
                        color: '#5f6368',
                        padding: { bottom: 20 }
                    },
                    legend: { display: false }
                },
                scales: {
                    y: { 
                        beginAtZero: true, 
                        grid: { color: '#e0e0e0', drawBorder: false },
                        ticks: { font: { size: 14 }, stepSize: 25 },
                        title: { display: true, text: 'Estimasi Memori (MB)', font: { size: 14 } }
                    },
                    x: { 
                        grid: { display: false },
                        ticks: { font: { size: 14 } },
                        title: { display: true, text: 'Struktur Data', font: { size: 14 }, padding: { top: 10 } }
                    }
                }
            }
        });
    </script>
</body>
</html>
)";
}
