// storage.cpp — Implementasi disk I/O dan operasi indeks.
// Menggunakan std::filesystem (C++17) — tidak ada system("dir").

#include "storage.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

#include <dirent.h>
#include <sys/stat.h>

// ============================================================
// Helper — estimasi ukuran string di heap
// ============================================================
static size_t str_heap(const std::string& s) {
    return sizeof(std::string) + (s.capacity() > 15 ? s.capacity() + 1 : 0);
}

static size_t satu_data_bytes(const DataArsip& d) {
    return sizeof(DataArsip)
         + str_heap(d.id) + str_heap(d.namaFile) + str_heap(d.tanggal)
         + str_heap(d.sumber) + str_heap(d.konten);
}

// ============================================================
// CRUD Indeks
// ============================================================
void tambah_ke_indeks(ArsipIndex& idx, const DataArsip& data) {
    idx.hashByNama[data.namaFile]  = data;
    idx.hashById[data.id]          = data;
    idx.hashByUkuran[data.ukuran].push_back(data.namaFile);
    idx.bstByNama[data.namaFile]   = data;
    idx.bstByUkuran[data.ukuran].push_back(data.namaFile);
    idx.files.push_back(data);
}

void hapus_dari_indeks(ArsipIndex& idx, const std::string& nama_file) {
    auto it = idx.hashByNama.find(nama_file);
    if (it == idx.hashByNama.end()) return;

    long long ukuran = it->second.ukuran;
    std::string id   = it->second.id;

    // Hash
    auto itu = idx.hashByUkuran.find(ukuran);
    if (itu != idx.hashByUkuran.end()) {
        auto& v = itu->second;
        v.erase(std::remove(v.begin(), v.end(), nama_file), v.end());
        if (v.empty()) idx.hashByUkuran.erase(itu);
    }
    idx.hashByNama.erase(it);
    idx.hashById.erase(id);

    // BST
    idx.bstByNama.erase(nama_file);
    auto ibu = idx.bstByUkuran.find(ukuran);
    if (ibu != idx.bstByUkuran.end()) {
        auto& v = ibu->second;
        v.erase(std::remove(v.begin(), v.end(), nama_file), v.end());
        if (v.empty()) idx.bstByUkuran.erase(ibu);
    }

    // Vector
    idx.files.erase(
        std::remove_if(idx.files.begin(), idx.files.end(),
                       [&](const DataArsip& d) { return d.namaFile == nama_file; }),
        idx.files.end()
    );
}

// ============================================================
// Search
// ============================================================
const DataArsip* cari_by_id(const ArsipIndex& idx, const std::string& id) {
    auto it = idx.hashById.find(id);
    return (it != idx.hashById.end()) ? &it->second : nullptr;
}

const DataArsip* cari_by_nama_hash(const ArsipIndex& idx, const std::string& nama) {
    auto it = idx.hashByNama.find(nama);
    return (it != idx.hashByNama.end()) ? &it->second : nullptr;
}

const DataArsip* cari_by_nama_bst(const ArsipIndex& idx, const std::string& nama) {
    auto it = idx.bstByNama.find(nama);
    return (it != idx.bstByNama.end()) ? &it->second : nullptr;
}

const DataArsip* cari_by_nama_vector(const ArsipIndex& idx, const std::string& nama) {
    for (const DataArsip& d : idx.files) {
        if (d.namaFile == nama) return &d;
    }
    return nullptr;
}

std::vector<DataArsip> cari_by_range_ukuran(const ArsipIndex& idx,
                                              long long min_bytes, long long max_bytes) {
    std::vector<DataArsip> hasil;
    auto lo = idx.bstByUkuran.lower_bound(min_bytes);
    auto hi = idx.bstByUkuran.upper_bound(max_bytes);
    for (auto it = lo; it != hi; ++it) {
        for (const std::string& nama : it->second) {
            auto it2 = idx.bstByNama.find(nama);
            if (it2 != idx.bstByNama.end()) hasil.push_back(it2->second);
        }
    }
    return hasil;
}

std::vector<DataArsip> cari_by_range_ukuran_vector(const ArsipIndex& idx,
                                                     long long min_bytes, long long max_bytes) {
    std::vector<DataArsip> hasil;
    for (const DataArsip& d : idx.files) {
        if (d.ukuran >= min_bytes && d.ukuran <= max_bytes) hasil.push_back(d);
    }
    return hasil;
}

// ============================================================
// Estimasi Memori
// ============================================================
size_t estimasi_memori_hash(const ArsipIndex& idx) {
    size_t total = sizeof(idx.hashByNama) + sizeof(idx.hashById) + sizeof(idx.hashByUkuran);
    total += idx.hashByNama.bucket_count()   * sizeof(void*);
    total += idx.hashById.bucket_count()     * sizeof(void*);
    total += idx.hashByUkuran.bucket_count() * sizeof(void*);
    for (const auto& kv : idx.hashByNama)   total += str_heap(kv.first) + satu_data_bytes(kv.second);
    for (const auto& kv : idx.hashById)     total += str_heap(kv.first) + satu_data_bytes(kv.second);
    for (const auto& kv : idx.hashByUkuran) {
        total += sizeof(long long) + sizeof(std::vector<std::string>)
               + kv.second.capacity() * sizeof(std::string);
        for (const std::string& s : kv.second) total += str_heap(s);
    }
    return total;
}

size_t estimasi_memori_bst(const ArsipIndex& idx) {
    const size_t node_overhead = 4 * sizeof(void*);  // parent, left, right, color
    size_t total = sizeof(idx.bstByNama) + sizeof(idx.bstByUkuran);
    for (const auto& kv : idx.bstByNama)
        total += node_overhead + str_heap(kv.first) + satu_data_bytes(kv.second);
    for (const auto& kv : idx.bstByUkuran) {
        total += node_overhead + sizeof(long long)
               + sizeof(std::vector<std::string>) + kv.second.capacity() * sizeof(std::string);
        for (const std::string& s : kv.second) total += str_heap(s);
    }
    return total;
}

size_t estimasi_memori_vector(const ArsipIndex& idx) {
    size_t total = sizeof(idx.files) + idx.files.capacity() * sizeof(DataArsip);
    for (const DataArsip& d : idx.files) total += satu_data_bytes(d);
    return total;
}

// ============================================================
// File I/O
// ============================================================
bool tulis_file(const std::string& folder, const std::string& nama_file,
                const std::string& isi) {
    buat_folder(folder);
    std::ofstream out(gabung_path(folder, nama_file), std::ios::binary);
    if (!out.is_open()) return false;
    out << isi;
    return true;
}

void load_data_ke_memori(ArsipIndex& idx) {
    idx.hashByNama.clear();  idx.hashById.clear();    idx.hashByUkuran.clear();
    idx.bstByNama.clear();   idx.bstByUkuran.clear(); idx.files.clear();
    idx.counter = 1;

    buat_folder(idx.folder);

    int loaded = 0;
    try {
        DIR* dir = opendir(idx.folder.c_str());
        if (dir != nullptr) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_type != DT_REG) continue;

                std::string nama_file = entry->d_name;
                std::string file_path = gabung_path(idx.folder, nama_file);

                std::ifstream file(file_path, std::ios::binary | std::ios::ate);
                if (!file.is_open()) continue;

                long long ukuran = static_cast<long long>(file.tellg());
                file.clear();
                file.seekg(0, std::ios::beg);
                std::string isi((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());

                DataArsip d;
                d.id       = "D" + std::to_string(idx.counter++);
                d.namaFile = nama_file;
                d.ukuran   = ukuran;
                d.tanggal  = cek_tanggal(file_path);
                d.sumber   = idx.folder;
                d.konten   = isi;

                tambah_ke_indeks(idx, d);
                ++loaded;
            }
            closedir(dir);
        }
    } catch (...) {
        std::cerr << "[!] Gagal membaca direktori: " << idx.folder << "\n";
    }

    std::cout << "[System] Berhasil memuat " << loaded
              << " file ke Hash Table, BST, dan Vector.\n";
}

// ============================================================
// Generate Data Dummy
// ============================================================
void populate_data(ArsipIndex& idx, const std::string& folder,
                   int jumlah, int persen_duplikat) {
    if (persen_duplikat < 0)   persen_duplikat = 0;
    if (persen_duplikat > 100) persen_duplikat = 100;

    try {
        std::string cmd = "rmdir /s /q \"" + folder + "\" 2> nul";
        system(cmd.c_str());
        buat_folder(folder);
    } catch (...) {
        std::cerr << "[!] Error membuat folder.\n";
        return;
    }

    int min_kb = (jumlah >= 5000) ? 10 : 100;
    int max_kb = (jumlah >= 5000) ? 30 : 300;

    std::cout << "\n[System] Generate " << jumlah << " file dummy ("
              << min_kb << "KB-" << max_kb << "KB, " << persen_duplikat << "% duplikat)...\n";
    if (jumlah >= 5000)
        std::cout << "[System] Dataset besar, proses dapat memakan waktu beberapa saat.\n";

    srand(static_cast<unsigned int>(time(nullptr)));

    // Template teks pengisi file
    std::string chunk = "DATA_ARSIP_DUMMY_";
    while (static_cast<int>(chunk.size()) < 10240)
        chunk += "TEKS_ACAK_UNTUK_MEMENUHI_UKURAN_FILE_ARSIP_DIGITAL_";

    std::vector<std::string> konten_unik;  // Simpan konten file unik untuk di-copy
    int jumlah_duplikat  = 0;
    int progress_interval = std::max(1, jumlah / 10);

    for (int i = 1; i <= jumlah; i++) {
        std::string nama_file = "arsip_data_" + std::to_string(i) + ".txt";
        std::string file_path = gabung_path(folder, nama_file);

        bool buat_duplikat = (i > 1) && ((rand() % 100) < persen_duplikat)
                             && !konten_unik.empty();

        if (buat_duplikat) {
            std::string isi = konten_unik[rand() % static_cast<int>(konten_unik.size())];
            std::ofstream dst(file_path, std::ios::binary);
            if (dst.is_open()) { dst << isi; ++jumlah_duplikat; }
        } else {
            int target_size = (min_kb * 1024) + (rand() % ((max_kb - min_kb + 1) * 1024));
            std::ostringstream oss;
            int current = 0;
            while (current < target_size) {
                int rem = target_size - current;
                if (rem < static_cast<int>(chunk.size())) { oss << chunk.substr(0, rem); current += rem; }
                else { oss << chunk; current += static_cast<int>(chunk.size()); }
            }
            oss << "ID_UNIK_" << i << "_" << rand();
            std::string isi = oss.str();
            std::ofstream file(file_path, std::ios::binary);
            if (file.is_open()) { file << isi; konten_unik.push_back(isi); }
        }

        if (i % progress_interval == 0 || i == jumlah)
            std::cout << " -> Berhasil memproses " << i << " file...\n";
    }

    std::cout << "\n[System] Selesai. Dari " << jumlah << " file, terdapat sekitar "
              << jumlah_duplikat << " file duplikat.\n";
}
