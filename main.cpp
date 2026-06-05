// main.cpp — Menu loop utama. Tidak ada logika bisnis di sini.

#include "arsip.hpp"
#include "detection.hpp"
#include "experiment.hpp"
#include "storage.hpp"
#include "utils.hpp"

#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <iomanip>

static const char* MENU_TEXT =
    "\n=== SISTEM DETEKSI DUPLIKASI DATA ===\n"
    "\n"
    "[ MANAJEMEN DATA ]\n"
    " 1. Insert data baru\n"
    " 2. Search data (by ID | Nama | Range Ukuran)\n"
    " 3. Update data\n"
    " 4. Delete data\n"
    " 5. Statistik lengkap\n"
    "\n"
    "[ DETEKSI DUPLIKASI ]\n"
    " 6. By konten     - Hash Table\n"
    " 7. By konten     - BST (Ordered Map)\n"
    " 8. By konten     - Vector Linear Search\n"
    " 9. By konten     - Bandingkan 3 metode\n"
    "10. By metadata   - Bandingkan 3 metode\n"
    "\n"
    "[ EKSPERIMEN ]\n"
    "11. Perbandingan performa (data saat ini)\n"
    "12. Eksperimen otomatis (multi-dataset + multi-skenario)\n"
    "13. Generate dummy data manual\n"
    "\n"
    "-9. Keluar\n"
    "Select : ";

int main() {
    ArsipIndex idx;
    std::vector<CatatanPerforma> riwayat;

    buat_folder(idx.folder);
    load_data_ke_memori(idx);

    int pilihan = 0;
    while (pilihan != -9) {
        std::cout << MENU_TEXT;
        if (!(std::cin >> pilihan)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (pilihan) {
        // ======== 1. INSERT ========
        case 1: {
            std::string nama, isi, folder;
            std::cout << "Input nama file: ";
            std::cin >> nama;
            std::cout << "Input isi file: ";
            std::getline(std::cin >> std::ws, isi);
            std::cout << "Input nama folder (Enter untuk default " << idx.folder << "): ";
            std::getline(std::cin, folder);
            if (folder.empty()) folder = idx.folder;

            if (tulis_file(folder, nama, isi)) {
                hapus_dari_indeks(idx, nama);
                DataArsip d;
                d.id       = "D" + std::to_string(idx.counter++);
                d.namaFile = nama;
                d.ukuran   = static_cast<long long>(isi.size());
                d.tanggal  = cek_tanggal(gabung_path(folder, nama));
                d.sumber   = folder;
                d.konten   = isi;
                tambah_ke_indeks(idx, d);
                std::cout << "[System] File berhasil dibuat dan indeks diperbarui.\n";
            } else {
                std::cout << "[!] Error membuat file.\n";
            }
            break;
        }

        // ======== 2. SEARCH ========
        case 2: {
            std::cout << "\nSearch by: (1) ID  (2) Nama  (3) Range Ukuran -> ";
            int sub; std::cin >> sub;
            if (sub == 1) {
                std::string id;
                std::cout << "Input ID (contoh D1): "; std::cin >> id;
                const DataArsip* d = cari_by_id(idx, id);
                if (d) std::cout << "Ditemukan: " << d->namaFile << " | " << format_bytes(static_cast<size_t>(d->ukuran)) << "\n";
                else   std::cout << "[!] Tidak ditemukan.\n";
            } else if (sub == 2) {
                std::string nama;
                std::cout << "Input nama file: "; std::cin >> nama;

                auto t0 = std::chrono::high_resolution_clock::now();
                const DataArsip* h = cari_by_nama_hash(idx, nama);
                double th = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();

                t0 = std::chrono::high_resolution_clock::now();
                const DataArsip* b = cari_by_nama_bst(idx, nama);
                double tb = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();

                t0 = std::chrono::high_resolution_clock::now();
                const DataArsip* v = cari_by_nama_vector(idx, nama);
                double tv = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();

                if (h) std::cout << "Ditemukan: " << h->namaFile << " | " << format_bytes(static_cast<size_t>(h->ukuran)) << "\n";
                else   std::cout << "[!] Tidak ditemukan.\n";

                std::cout << std::fixed << std::setprecision(6);
                std::cout << "Waktu Hash  : " << th << " ms\n";
                std::cout << "Waktu BST   : " << tb << " ms\n";
                std::cout << "Waktu Vector: " << tv << " ms\n";
                (void)b; (void)v; // suppress unused
            } else if (sub == 3) {
                long long mn, mx;
                std::cout << "Min bytes: "; std::cin >> mn;
                std::cout << "Max bytes: "; std::cin >> mx;

                auto t0 = std::chrono::high_resolution_clock::now();
                auto rb = cari_by_range_ukuran(idx, mn, mx);
                double tb = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();

                t0 = std::chrono::high_resolution_clock::now();
                auto rv = cari_by_range_ukuran_vector(idx, mn, mx);
                double tv = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();

                std::cout << "Ditemukan " << rb.size() << " file.\n";
                int tampil = std::min(10, static_cast<int>(rb.size()));
                for (int i = 0; i < tampil; i++)
                    std::cout << " - " << rb[i].namaFile << " (" << format_bytes(static_cast<size_t>(rb[i].ukuran)) << ")\n";
                if (static_cast<int>(rb.size()) > tampil)
                    std::cout << " ... " << (rb.size() - tampil) << " file lain.\n";

                std::cout << std::fixed << std::setprecision(6);
                std::cout << "Waktu BST (lower/upper_bound): " << tb << " ms\n";
                std::cout << "Waktu Vector (linear scan)   : " << tv << " ms\n";
                (void)rv;
            }
            break;
        }

        // ======== 3. UPDATE ========
        case 3: {
            std::string nama;
            std::cout << "Input nama file yang akan di-update: ";
            std::cin >> nama;
            const DataArsip* d = cari_by_nama_hash(idx, nama);
            if (!d) { std::cout << "[!] File tidak ditemukan.\n"; break; }
            std::string isi_baru;
            std::cout << "Input isi file baru: ";
            std::getline(std::cin >> std::ws, isi_baru);
            if (tulis_file(d->sumber, nama, isi_baru)) {
                std::cout << "[System] File berhasil di-update. Memuat ulang indeks...\n";
                load_data_ke_memori(idx);
            } else {
                std::cout << "[!] Gagal update file fisik.\n";
            }
            break;
        }

        // ======== 4. DELETE ========
        case 4: {
            std::string nama;
            std::cout << "Input nama file yang akan dihapus: ";
            std::cin >> nama;
            const DataArsip* d = cari_by_nama_hash(idx, nama);
            if (!d) { std::cout << "[!] File tidak ditemukan.\n"; break; }
            std::string target = gabung_path(d->sumber, nama);
            if (std::remove(target.c_str()) == 0) {
                std::cout << "[System] File berhasil dihapus. Memuat ulang indeks...\n";
                load_data_ke_memori(idx);
            } else {
                std::cout << "[!] Gagal menghapus file fisik.\n";
            }
            break;
        }

        // ======== 5. STATISTIK ========
        case 5:
            tampilkan_statistik_lengkap(idx);
            break;

        // ======== 6–8. DETEKSI INDIVIDUAL ========
        case 6: {
            std::cout << "\n=== DETEKSI DUPLIKASI — HASH TABLE ===\n";
            if (idx.files.empty()) { std::cout << "[!] Memori kosong.\n"; break; }
            HasilDeteksi h = deteksi_hash(idx, true);
            tampilkan_ringkasan("Hash Table", h);
            break;
        }
        case 7: {
            std::cout << "\n=== DETEKSI DUPLIKASI — BST (ORDERED MAP) ===\n";
            if (idx.files.empty()) { std::cout << "[!] Memori kosong.\n"; break; }
            HasilDeteksi h = deteksi_bst(idx, true);
            tampilkan_ringkasan("BST (Ordered Map)", h);
            break;
        }
        case 8: {
            std::cout << "\n=== DETEKSI DUPLIKASI — VECTOR LINEAR SEARCH ===\n";
            if (idx.files.empty()) { std::cout << "[!] Memori kosong.\n"; break; }
            HasilDeteksi h = deteksi_vector(idx, true);
            tampilkan_ringkasan("Vector Linear Search", h);
            break;
        }

        // ======== 9. BANDINGKAN 3 METODE (konten) ========
        case 9: {
            if (idx.files.empty()) { std::cout << "[!] Memori kosong.\n"; break; }
            std::cout << "\n=== BANDINGKAN 3 METODE DETEKSI BY KONTEN ===\n";
            HasilDeteksi rH = deteksi_hash(idx, false);
            HasilDeteksi rB = deteksi_bst(idx, false);
            HasilDeteksi rV = deteksi_vector(idx, false);
            tampilkan_ringkasan("Hash Table", rH);
            tampilkan_ringkasan("BST (Ordered Map)", rB);
            tampilkan_ringkasan("Vector Linear Search", rV);
            break;
        }

        // ======== 10. METADATA ========
        case 10: {
            if (idx.files.empty()) { std::cout << "[!] Memori kosong.\n"; break; }
            std::cout << "\n=== DETEKSI BY METADATA (Nama+Ukuran) — 3 METODE ===\n";
            HasilDeteksi mH, mB, mV;
            deteksi_metadata(idx, true, mH, mB, mV);
            tampilkan_ringkasan("Hash (metadata)", mH);
            tampilkan_ringkasan("BST (metadata)", mB);
            tampilkan_ringkasan("Vector (metadata)", mV);
            break;
        }

        // ======== 11. PERBANDINGAN ========
        case 11:
            jalankan_perbandingan(idx, riwayat);
            break;

        // ======== 12. EKSPERIMEN OTOMATIS ========
        case 12:
            jalankan_eksperimen_otomatis(idx, riwayat);
            break;

        // ======== 13. GENERATE DUMMY ========
        case 13: {
            std::cout << "\nPilih Skala Dataset:\n";
            std::cout << " [ PENGEMBANGAN ] 1. 1,000 data\n";
            std::cout << "                  2. 10,000 data\n";
            std::cout << " [ OBSERVASI    ] 3. 100,000 data\n";
            std::cout << "                  4. 500,000 data\n";
            std::cout << " [ EKSTREM      ] 5. 1,000,000 data\n";
            std::cout << "                  6. 2,500,000 data\n";
            std::cout << "Pilih -> ";
            int sub; std::cin >> sub;
            int jumlah = 1000;
            if (sub == 2) jumlah = 10000;
            else if (sub == 3) jumlah = 100000;
            else if (sub == 4) jumlah = 500000;
            else if (sub == 5) jumlah = 1000000;
            else if (sub == 6) jumlah = 2500000;

            int persen = 15;
            std::cout << "Persen duplikat (rekomendasi 10-20, default 15): ";
            if (std::cin.peek() == '\n') std::cin.ignore();
            std::string tmp;
            std::getline(std::cin, tmp);
            if (!tmp.empty()) {
                try { persen = std::stoi(tmp); } catch (...) {}
            }

            populate_data(idx, idx.folder, jumlah, persen);
            std::cout << "[System] Memuat data dari folder ke RAM...\n";
            load_data_ke_memori(idx);
            break;
        }

        // ======== -9. KELUAR ========
        case -9:
            std::cout << "Menutup program...\n";
            break;

        default:
            std::cout << "Input tidak valid.\n";
            break;
        }
    }

    return 0;
}
