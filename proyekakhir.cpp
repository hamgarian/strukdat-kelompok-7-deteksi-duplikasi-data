#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <direct.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

using namespace std;

struct DataArsip {
    string id;
    string namaFile;
    long long int ukuran;
    string tanggal;
    string sumber;
    string konten;
};

struct GrupDuplikat {
    long long int ukuran;
    vector<string> namaFile;
};

struct HasilDeteksi {
    vector<GrupDuplikat> grup;
    double waktuMs;
    size_t estimasiMemoriBytes;
};

struct CatatanPerforma {
    int jumlahData;
    double waktuHashMs;
    double waktuVectorMs;
    size_t memoriHashBytes;
    size_t memoriVectorBytes;
    int jumlahGrupDuplikat;
    int jumlahFileDuplikat;
};

// Struktur data pembanding
unordered_map<long long int, vector<string>> file_map_ukuran;
unordered_map<string, DataArsip> file_map_nama;
vector<DataArsip> file_vector;

vector<CatatanPerforma> riwayat_performa;
long long int counter = 1;
string nama_folder = "arsip_digital";

string gabung_path(const string& folder, const string& nama_file) {
    if (folder.empty()) {
        return nama_file;
    }

    char akhir = folder[folder.size() - 1];
    if (akhir == '/' || akhir == '\\') {
        return folder + nama_file;
    }

    return folder + "/" + nama_file;
}

string format_bytes(size_t bytes) {
    const char* satuan[] = {"B", "KB", "MB", "GB"};
    double nilai = static_cast<double>(bytes);
    int idx = 0;

    while (nilai >= 1024.0 && idx < 3) {
        nilai /= 1024.0;
        idx++;
    }

    stringstream ss;
    ss << fixed << setprecision(idx == 0 ? 0 : 2) << nilai << " " << satuan[idx];
    return ss.str();
}

string cek_tanggal(const string& file_path) {
    struct stat info_file;
    if (stat(file_path.c_str(), &info_file) == 0) {
        time_t raw_time = info_file.st_mtime;
        struct tm* waktu_lokal = localtime(&raw_time);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", waktu_lokal);
        return string(buffer);
    }
    return "Unknown";
}

size_t kapasitas_string(const string& nilai) {
    return sizeof(string) + nilai.capacity() + 1;
}

size_t estimasi_memori_data(const DataArsip& data) {
    return sizeof(DataArsip)
        + data.id.capacity() + 1
        + data.namaFile.capacity() + 1
        + data.tanggal.capacity() + 1
        + data.sumber.capacity() + 1
        + data.konten.capacity() + 1;
}

size_t estimasi_memori_hash_table() {
    size_t total = sizeof(file_map_nama) + sizeof(file_map_ukuran);
    total += file_map_nama.bucket_count() * sizeof(void*);
    total += file_map_ukuran.bucket_count() * sizeof(void*);

    for (const auto& item : file_map_nama) {
        total += sizeof(item) + kapasitas_string(item.first) + estimasi_memori_data(item.second);
    }

    for (const auto& item : file_map_ukuran) {
        total += sizeof(item) + item.second.capacity() * sizeof(string);
        for (const string& nama : item.second) {
            total += kapasitas_string(nama);
        }
    }

    return total;
}

size_t estimasi_memori_vector() {
    size_t total = sizeof(file_vector) + file_vector.capacity() * sizeof(DataArsip);
    for (const DataArsip& data : file_vector) {
        total += estimasi_memori_data(data);
    }
    return total;
}

long long int total_ukuran_file() {
    long long int total = 0;
    for (const DataArsip& data : file_vector) {
        total += data.ukuran;
    }
    return total;
}

void pastikan_folder_ada(const string& folder) {
    _mkdir(folder.c_str());
}

void hapus_dari_indeks(const string& nama_file) {
    auto data_lama = file_map_nama.find(nama_file);
    if (data_lama != file_map_nama.end()) {
        long long int ukuran_lama = data_lama->second.ukuran;
        auto indeks_ukuran = file_map_ukuran.find(ukuran_lama);

        if (indeks_ukuran != file_map_ukuran.end()) {
            vector<string>& daftar_nama = indeks_ukuran->second;
            daftar_nama.erase(
                remove(daftar_nama.begin(), daftar_nama.end(), nama_file),
                daftar_nama.end()
            );

            if (daftar_nama.empty()) {
                file_map_ukuran.erase(indeks_ukuran);
            }
        }

        file_map_nama.erase(data_lama);
    }

    file_vector.erase(
        remove_if(
            file_vector.begin(),
            file_vector.end(),
            [&](const DataArsip& data) {
                return data.namaFile == nama_file;
            }
        ),
        file_vector.end()
    );
}

bool tulis_file(const string& folder, const string& nama_file, const string& isi) {
    pastikan_folder_ada(folder);
    string target = gabung_path(folder, nama_file);
    ofstream data(target, ios::binary);

    if (!data.is_open()) {
        return false;
    }

    data << isi;
    return true;
}

void tambah_ke_memori(const string& nama_file, const string& folder, const string& isi) {
    string file_path = gabung_path(folder, nama_file);

    DataArsip data_baru;
    data_baru.id = "D" + to_string(counter++);
    data_baru.namaFile = nama_file;
    data_baru.ukuran = static_cast<long long int>(isi.size());
    data_baru.tanggal = cek_tanggal(file_path);
    data_baru.sumber = folder;
    data_baru.konten = isi;

    file_map_ukuran[data_baru.ukuran].push_back(nama_file);
    file_map_nama[nama_file] = data_baru;
    file_vector.push_back(data_baru);
}

void insert_data(const string& nama_file, const string& isi, const string& folder) {
    if (tulis_file(folder, nama_file, isi)) {
        hapus_dari_indeks(nama_file);
        tambah_ke_memori(nama_file, folder, isi);
        cout << "[System] File berhasil dibuat dan indeks memori diperbarui.\n";
    } else {
        cout << "[!] Error membuat file.\n";
    }
}

void load_data_ke_memori() {
    file_map_ukuran.clear();
    file_map_nama.clear();
    file_vector.clear();
    counter = 1;

    pastikan_folder_ada(nama_folder);

    string command = "dir \"" + nama_folder + "\" /b /a-d > file_list.txt 2> nul";
    system(command.c_str());

    ifstream file_list("file_list.txt");
    string nama_file;

    while (getline(file_list, nama_file)) {
        if (!nama_file.empty() && nama_file[nama_file.size() - 1] == '\r') {
            nama_file.erase(nama_file.size() - 1);
        }

        string file_path = gabung_path(nama_folder, nama_file);
        ifstream file(file_path, ios::binary | ios::ate);

        if (!file.is_open()) {
            continue;
        }

        long long int ukuran_file = static_cast<long long int>(file.tellg());
        file.clear();
        file.seekg(0, ios::beg);
        string isi_konten((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

        DataArsip data_arsip;
        data_arsip.id = "D" + to_string(counter++);
        data_arsip.namaFile = nama_file;
        data_arsip.ukuran = ukuran_file;
        data_arsip.tanggal = cek_tanggal(file_path);
        data_arsip.sumber = nama_folder;
        data_arsip.konten = isi_konten;

        file_map_ukuran[ukuran_file].push_back(nama_file);
        file_map_nama[nama_file] = data_arsip;
        file_vector.push_back(data_arsip);
    }

    file_list.close();
    remove("file_list.txt");

    cout << "[System] Berhasil memuat " << file_vector.size()
         << " file ke Hash Table dan Vector.\n";
}

void populate_data(const string& folder, int jumlah_data) {
    string hapus_cmd = "rmdir /s /q \"" + folder + "\" 2> nul";
    system(hapus_cmd.c_str());
    pastikan_folder_ada(folder);

    int min_kb = jumlah_data >= 5000 ? 10 : 100;
    int max_kb = jumlah_data >= 5000 ? 30 : 300;
    int progress_interval = max(500, jumlah_data / 10);

    cout << "\n[System] Generate " << jumlah_data << " file dummy ("
         << min_kb << "KB - " << max_kb << "KB per file)...\n";
    if (jumlah_data >= 5000) {
        cout << "[System] Dataset besar, proses dapat memakan waktu beberapa saat.\n";
    }

    srand(static_cast<unsigned int>(time(0)));

    string chunk = "DATA_ARSIP_DUMMY_";
    while (chunk.length() < 10240) {
        chunk += "TEKS_ACAK_UNTUK_MEMENUHI_UKURAN_FILE_ARSIP_DIGITAL_";
    }

    int jumlah_duplikat = 0;

    for (int i = 1; i <= jumlah_data; i++) {
        string nama_file = "arsip_data_" + to_string(i) + ".txt";
        string file_path = gabung_path(folder, nama_file);
        int chance = rand() % 100;

        if (i > 1 && chance < 10) {
            int target_copy = (rand() % (i - 1)) + 1;
            string source_path = gabung_path(folder, "arsip_data_" + to_string(target_copy) + ".txt");

            ifstream src(source_path, ios::binary);
            ofstream dst(file_path, ios::binary);
            if (src.is_open() && dst.is_open()) {
                dst << src.rdbuf();
                jumlah_duplikat++;
            }
        } else {
            int target_size = (min_kb * 1024) + (rand() % ((max_kb - min_kb + 1) * 1024));
            ofstream file(file_path, ios::binary);

            if (file.is_open()) {
                int current_size = 0;
                while (current_size < target_size) {
                    int remaining = target_size - current_size;
                    if (remaining < static_cast<int>(chunk.length())) {
                        file << chunk.substr(0, remaining);
                        current_size += remaining;
                    } else {
                        file << chunk;
                        current_size += static_cast<int>(chunk.length());
                    }
                }
                file << "ID_UNIK_" << i << "_" << rand();
            }
        }

        if (i % progress_interval == 0 || i == jumlah_data) {
            cout << " -> Berhasil memproses " << i << " file...\n";
        }
    }

    cout << "\n[System] Selesai. Dari " << jumlah_data << " file, terdapat sekitar "
         << jumlah_duplikat << " file duplikat yang tercipta secara acak.\n";
}

void cetak_grup_duplikat(const vector<GrupDuplikat>& grup, int batas = 20) {
    if (grup.empty()) {
        cout << "Tidak ada duplikasi yang ditemukan.\n";
        return;
    }

    int tampil = min(static_cast<int>(grup.size()), batas);
    for (int i = 0; i < tampil; i++) {
        cout << "-> DUPLIKAT (Ukuran: " << grup[i].ukuran << " Bytes) | File: ";
        for (const string& nama : grup[i].namaFile) {
            cout << nama << " ";
        }
        cout << "\n";
    }

    if (static_cast<int>(grup.size()) > batas) {
        cout << "... " << (grup.size() - batas)
             << " grup duplikat lain disembunyikan agar output tetap ringkas.\n";
    }
}

HasilDeteksi deteksi_duplikasi_hash_table(bool tampilkan_detail) {
    auto start_time = chrono::high_resolution_clock::now();
    vector<GrupDuplikat> hasil;

    for (const auto& item_ukuran : file_map_ukuran) {
        if (item_ukuran.second.size() <= 1) {
            continue;
        }

        unordered_map<string, vector<string>> map_konten_lokal;
        for (const string& nama : item_ukuran.second) {
            auto data = file_map_nama.find(nama);
            if (data != file_map_nama.end()) {
                map_konten_lokal[data->second.konten].push_back(nama);
            }
        }

        for (const auto& item_konten : map_konten_lokal) {
            if (item_konten.second.size() > 1) {
                hasil.push_back({item_ukuran.first, item_konten.second});
            }
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> durasi = end_time - start_time;

    if (tampilkan_detail) {
        cetak_grup_duplikat(hasil);
    }

    return {hasil, durasi.count(), estimasi_memori_hash_table()};
}

HasilDeteksi deteksi_duplikasi_vector_linear(bool tampilkan_detail) {
    struct GrupLinear {
        long long int ukuran;
        string kontenPembanding;
        vector<string> namaFile;
    };

    auto start_time = chrono::high_resolution_clock::now();
    vector<GrupLinear> grup_linear;

    for (const DataArsip& data : file_vector) {
        bool ditemukan = false;

        for (GrupLinear& grup : grup_linear) {
            if (grup.ukuran == data.ukuran && grup.kontenPembanding == data.konten) {
                grup.namaFile.push_back(data.namaFile);
                ditemukan = true;
                break;
            }
        }

        if (!ditemukan) {
            grup_linear.push_back({data.ukuran, data.konten, {data.namaFile}});
        }
    }

    vector<GrupDuplikat> hasil;
    for (const GrupLinear& grup : grup_linear) {
        if (grup.namaFile.size() > 1) {
            hasil.push_back({grup.ukuran, grup.namaFile});
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> durasi = end_time - start_time;

    if (tampilkan_detail) {
        cetak_grup_duplikat(hasil);
    }

    return {hasil, durasi.count(), estimasi_memori_vector()};
}

int hitung_file_duplikat(const vector<GrupDuplikat>& grup) {
    int total = 0;
    for (const GrupDuplikat& item : grup) {
        total += static_cast<int>(item.namaFile.size());
    }
    return total;
}

void tampilkan_ringkasan_deteksi(const string& label, const HasilDeteksi& hasil) {
    cout << "------------------------------------------\n";
    cout << "[!] METODE              : " << label << "\n";
    cout << "[!] WAKTU EKSEKUSI      : " << fixed << setprecision(4) << hasil.waktuMs << " ms\n";
    cout << "[!] ESTIMASI MEMORI     : " << format_bytes(hasil.estimasiMemoriBytes) << "\n";
    cout << "[!] GRUP DUPLIKAT       : " << hasil.grup.size() << "\n";
    cout << "[!] FILE DALAM DUPLIKAT : " << hitung_file_duplikat(hasil.grup) << "\n";
    cout << "------------------------------------------\n";
}

string bar(double nilai, double nilai_maks, int lebar = 45) {
    if (nilai_maks <= 0.0) {
        return "";
    }

    int panjang = static_cast<int>((nilai / nilai_maks) * lebar);
    panjang = max(1, min(lebar, panjang));
    return string(panjang, '#');
}

void tampilkan_grafik_console(const vector<CatatanPerforma>& data) {
    if (data.empty()) {
        cout << "[!] Belum ada data eksperimen. Jalankan menu eksperimen terlebih dahulu.\n";
        return;
    }

    double waktu_maks = 0.0;
    size_t memori_maks = 0;
    for (const CatatanPerforma& item : data) {
        waktu_maks = max(waktu_maks, max(item.waktuHashMs, item.waktuVectorMs));
        memori_maks = max(memori_maks, max(item.memoriHashBytes, item.memoriVectorBytes));
    }

    cout << "\n=== Grafik Waktu Eksekusi (ms) ===\n";
    for (const CatatanPerforma& item : data) {
        cout << "[" << setw(5) << item.jumlahData << "] Hash   | "
             << bar(item.waktuHashMs, waktu_maks) << " "
             << fixed << setprecision(3) << item.waktuHashMs << " ms\n";
        cout << "[" << setw(5) << item.jumlahData << "] Vector | "
             << bar(item.waktuVectorMs, waktu_maks) << " "
             << fixed << setprecision(3) << item.waktuVectorMs << " ms\n";
    }

    cout << "\n=== Grafik Estimasi Memori ===\n";
    for (const CatatanPerforma& item : data) {
        cout << "[" << setw(5) << item.jumlahData << "] Hash   | "
             << bar(static_cast<double>(item.memoriHashBytes), static_cast<double>(memori_maks))
             << " " << format_bytes(item.memoriHashBytes) << "\n";
        cout << "[" << setw(5) << item.jumlahData << "] Vector | "
             << bar(static_cast<double>(item.memoriVectorBytes), static_cast<double>(memori_maks))
             << " " << format_bytes(item.memoriVectorBytes) << "\n";
    }
}

void simpan_csv(const vector<CatatanPerforma>& data) {
    ofstream file("performance_report.csv");
    file << "jumlah_data,waktu_hash_ms,waktu_vector_ms,memori_hash_bytes,memori_vector_bytes,grup_duplikat,file_duplikat\n";
    for (const CatatanPerforma& item : data) {
        file << item.jumlahData << ","
             << fixed << setprecision(6) << item.waktuHashMs << ","
             << item.waktuVectorMs << ","
             << item.memoriHashBytes << ","
             << item.memoriVectorBytes << ","
             << item.jumlahGrupDuplikat << ","
             << item.jumlahFileDuplikat << "\n";
    }
}

void simpan_grafik_html(const vector<CatatanPerforma>& data) {
    if (data.empty()) {
        return;
    }

    double waktu_maks = 0.0;
    size_t memori_maks = 0;
    for (const CatatanPerforma& item : data) {
        waktu_maks = max(waktu_maks, max(item.waktuHashMs, item.waktuVectorMs));
        memori_maks = max(memori_maks, max(item.memoriHashBytes, item.memoriVectorBytes));
    }

    ofstream html("grafik_performa.html");
    html << "<!doctype html><html><head><meta charset=\"utf-8\"><title>Grafik Performa</title>";
    html << "<style>body{font-family:Arial,sans-serif;margin:32px;color:#202124}"
         << ".row{display:grid;grid-template-columns:90px 80px 1fr 140px;gap:10px;align-items:center;margin:8px 0}"
         << ".bar{height:22px;background:#0f766e}.bar.vector{background:#c2410c}"
         << "h1,h2{margin-top:28px}.note{color:#5f6368}</style></head><body>";
    html << "<h1>Grafik Perbandingan Performa</h1>";
    html << "<p class=\"note\">Hash Table dibandingkan dengan Vector Linear Search.</p>";

    html << "<h2>Waktu Eksekusi</h2>";
    for (const CatatanPerforma& item : data) {
        double hash_width = waktu_maks > 0 ? (item.waktuHashMs / waktu_maks) * 100.0 : 0.0;
        double vector_width = waktu_maks > 0 ? (item.waktuVectorMs / waktu_maks) * 100.0 : 0.0;
        html << "<div class=\"row\"><b>" << item.jumlahData << " data</b><span>Hash</span><div class=\"bar\" style=\"width:"
             << hash_width << "%\"></div><span>" << fixed << setprecision(3) << item.waktuHashMs << " ms</span></div>";
        html << "<div class=\"row\"><b>" << item.jumlahData << " data</b><span>Vector</span><div class=\"bar vector\" style=\"width:"
             << vector_width << "%\"></div><span>" << fixed << setprecision(3) << item.waktuVectorMs << " ms</span></div>";
    }

    html << "<h2>Estimasi Memori</h2>";
    for (const CatatanPerforma& item : data) {
        double hash_width = memori_maks > 0 ? (static_cast<double>(item.memoriHashBytes) / memori_maks) * 100.0 : 0.0;
        double vector_width = memori_maks > 0 ? (static_cast<double>(item.memoriVectorBytes) / memori_maks) * 100.0 : 0.0;
        html << "<div class=\"row\"><b>" << item.jumlahData << " data</b><span>Hash</span><div class=\"bar\" style=\"width:"
             << hash_width << "%\"></div><span>" << format_bytes(item.memoriHashBytes) << "</span></div>";
        html << "<div class=\"row\"><b>" << item.jumlahData << " data</b><span>Vector</span><div class=\"bar vector\" style=\"width:"
             << vector_width << "%\"></div><span>" << format_bytes(item.memoriVectorBytes) << "</span></div>";
    }

    html << "</body></html>";
}

CatatanPerforma buat_catatan_performa(int jumlah_data, const HasilDeteksi& hash, const HasilDeteksi& vector) {
    return {
        jumlah_data,
        hash.waktuMs,
        vector.waktuMs,
        hash.estimasiMemoriBytes,
        vector.estimasiMemoriBytes,
        static_cast<int>(hash.grup.size()),
        hitung_file_duplikat(hash.grup)
    };
}

void jalankan_perbandingan_data_saat_ini() {
    if (file_vector.empty()) {
        cout << "[!] Memori kosong. Generate atau insert data terlebih dahulu.\n";
        return;
    }

    cout << "\n=== PERBANDINGAN HASH TABLE VS VECTOR LINEAR SEARCH ===\n";
    HasilDeteksi hash = deteksi_duplikasi_hash_table(false);
    HasilDeteksi vector = deteksi_duplikasi_vector_linear(false);

    riwayat_performa.clear();
    riwayat_performa.push_back(buat_catatan_performa(static_cast<int>(file_vector.size()), hash, vector));

    tampilkan_ringkasan_deteksi("Hash Table", hash);
    tampilkan_ringkasan_deteksi("Vector Linear Search", vector);
    tampilkan_grafik_console(riwayat_performa);
    simpan_csv(riwayat_performa);
    simpan_grafik_html(riwayat_performa);

    cout << "[System] Laporan disimpan ke performance_report.csv dan grafik_performa.html.\n";
}

void jalankan_eksperimen_otomatis() {
    vector<int> ukuran_dataset = {100, 1000, 5000, 10000};
    riwayat_performa.clear();

    cout << "\n=== EKSPERIMEN OTOMATIS DATASET BESAR ===\n";
    for (int jumlah : ukuran_dataset) {
        cout << "\n[Experiment] Dataset " << jumlah << " data\n";
        populate_data(nama_folder, jumlah);
        load_data_ke_memori();

        HasilDeteksi hash = deteksi_duplikasi_hash_table(false);
        HasilDeteksi vector = deteksi_duplikasi_vector_linear(false);
        CatatanPerforma catatan = buat_catatan_performa(jumlah, hash, vector);
        riwayat_performa.push_back(catatan);

        cout << "Hash   : " << fixed << setprecision(4) << hash.waktuMs
             << " ms | Memori " << format_bytes(hash.estimasiMemoriBytes) << "\n";
        cout << "Vector : " << fixed << setprecision(4) << vector.waktuMs
             << " ms | Memori " << format_bytes(vector.estimasiMemoriBytes) << "\n";
        cout << "Duplikat: " << catatan.jumlahGrupDuplikat << " grup, "
             << catatan.jumlahFileDuplikat << " file.\n";
    }

    tampilkan_grafik_console(riwayat_performa);
    simpan_csv(riwayat_performa);
    simpan_grafik_html(riwayat_performa);
    cout << "\n[System] Laporan eksperimen disimpan ke performance_report.csv dan grafik_performa.html.\n";
}

void update_data() {
    string nama_file;
    cout << "Input nama file yang akan di-update: ";
    cin >> nama_file;

    auto data = file_map_nama.find(nama_file);
    if (data == file_map_nama.end()) {
        cout << "[!] File tidak ditemukan di memori.\n";
        return;
    }

    string isi_baru;
    cout << "Input isi file baru: ";
    getline(cin >> ws, isi_baru);

    if (!tulis_file(data->second.sumber, nama_file, isi_baru)) {
        cout << "[!] Gagal update file fisik.\n";
        return;
    }

    cout << "[System] File berhasil di-update. Memuat ulang indeks...\n";
    load_data_ke_memori();
}

void delete_data() {
    string nama_file;
    cout << "Input nama file yang akan dihapus: ";
    cin >> nama_file;

    auto data = file_map_nama.find(nama_file);
    if (data == file_map_nama.end()) {
        cout << "[!] File tidak ditemukan di memori.\n";
        return;
    }

    string target = gabung_path(data->second.sumber, nama_file);
    if (remove(target.c_str()) == 0) {
        cout << "[System] File berhasil dihapus. Memuat ulang indeks...\n";
        load_data_ke_memori();
    } else {
        cout << "[!] Gagal menghapus file fisik.\n";
    }
}

void tampilkan_statistik() {
    cout << "\n=== STATISTIK DATA ARSIP ===\n";
    cout << "Jumlah file              : " << file_vector.size() << "\n";
    cout << "Total ukuran file        : " << format_bytes(static_cast<size_t>(total_ukuran_file())) << "\n";
    cout << "Estimasi memori Hash     : " << format_bytes(estimasi_memori_hash_table()) << "\n";
    cout << "Estimasi memori Vector   : " << format_bytes(estimasi_memori_vector()) << "\n";

    if (file_vector.empty()) {
        return;
    }

    auto minmax = minmax_element(
        file_vector.begin(),
        file_vector.end(),
        [](const DataArsip& a, const DataArsip& b) {
            return a.ukuran < b.ukuran;
        }
    );

    double rata_rata = static_cast<double>(total_ukuran_file()) / file_vector.size();
    HasilDeteksi hash = deteksi_duplikasi_hash_table(false);

    cout << "Rata-rata ukuran file    : " << format_bytes(static_cast<size_t>(rata_rata)) << "\n";
    cout << "File terkecil            : " << minmax.first->namaFile << " (" << format_bytes(static_cast<size_t>(minmax.first->ukuran)) << ")\n";
    cout << "File terbesar            : " << minmax.second->namaFile << " (" << format_bytes(static_cast<size_t>(minmax.second->ukuran)) << ")\n";
    cout << "Grup duplikat            : " << hash.grup.size() << "\n";
    cout << "File yang masuk duplikat : " << hitung_file_duplikat(hash.grup) << "\n";
}

int main() {
    pastikan_folder_ada(nama_folder);
    load_data_ke_memori();

    string menu_text = "\n=== User Menu ===\n"
                       "1. Insert Data Baru\n"
                       "2. Deteksi Duplikasi - Hash Table\n"
                       "3. Deteksi Duplikasi - Vector Linear Search\n"
                       "4. Perbandingan Hash vs Vector pada Data Saat Ini\n"
                       "5. Eksperimen Otomatis Dataset Besar + Grafik\n"
                       "6. Generate Dummy Data Manual\n"
                       "7. Update Data\n"
                       "8. Delete Data\n"
                       "9. Statistik Sederhana\n"
                       "10. Tampilkan Grafik Performa Terakhir\n"
                       "-9. Close\n"
                       "Select : ";

    int menu_input = 0;
    while (menu_input != -9) {
        cout << menu_text;
        if (!(cin >> menu_input)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch(menu_input) {
            case 1: {
                string input_nama_file, isi, input_folder;
                cout << "Input nama file: ";
                cin >> input_nama_file;
                cout << "Input isi file: ";
                getline(cin >> ws, isi);
                cout << "Input nama folder (- untuk default arsip_digital): ";
                cin >> input_folder;
                if (input_folder == "-") {
                    input_folder = nama_folder;
                }

                insert_data(input_nama_file, isi, input_folder);
                break;
            }
            case 2: {
                cout << "\n=== DETEKSI DUPLIKASI HASH TABLE ===\n";
                if (file_map_nama.empty()) {
                    cout << "[!] Memori kosong. Generate atau insert data terlebih dahulu.\n";
                    break;
                }
                HasilDeteksi hasil = deteksi_duplikasi_hash_table(true);
                tampilkan_ringkasan_deteksi("Hash Table", hasil);
                break;
            }
            case 3: {
                cout << "\n=== DETEKSI DUPLIKASI VECTOR LINEAR SEARCH ===\n";
                if (file_vector.empty()) {
                    cout << "[!] Memori kosong. Generate atau insert data terlebih dahulu.\n";
                    break;
                }
                HasilDeteksi hasil = deteksi_duplikasi_vector_linear(true);
                tampilkan_ringkasan_deteksi("Vector Linear Search", hasil);
                break;
            }
            case 4:
                jalankan_perbandingan_data_saat_ini();
                break;
            case 5:
                jalankan_eksperimen_otomatis();
                break;
            case 6: {
                cout << "\nJumlah data: (1) 100  (2) 1000  (3) 5000  (4) 10000 -> Pilih: ";
                int pilihan;
                cin >> pilihan;

                int jumlah = 100;
                if (pilihan == 2) jumlah = 1000;
                else if (pilihan == 3) jumlah = 5000;
                else if (pilihan == 4) jumlah = 10000;

                populate_data(nama_folder, jumlah);
                cout << "[System] Memperbarui Hash Table dan Vector di RAM...\n";
                load_data_ke_memori();
                break;
            }
            case 7:
                update_data();
                break;
            case 8:
                delete_data();
                break;
            case 9:
                tampilkan_statistik();
                break;
            case 10:
                tampilkan_grafik_console(riwayat_performa);
                if (!riwayat_performa.empty()) {
                    simpan_csv(riwayat_performa);
                    simpan_grafik_html(riwayat_performa);
                    cout << "[System] Grafik juga tersimpan di grafik_performa.html.\n";
                }
                break;
            case -9:
                cout << "Menutup program...\n";
                break;
            default:
                cout << "Input tidak valid.\n";
                break;
        }
    }

    return 0;
}
