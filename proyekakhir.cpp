#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <ctime>
#include <cstdlib>
#include <chrono>

using namespace std;

struct DataArsip {
    string id;
    string namaFile;
    long long int ukuran;
    string tanggal;
    string sumber;
    string konten;
};

// Variabel Global untuk Memori (Agar bisa di-update otomatis)
unordered_map<long long int, vector<string>> file_map_ukuran;
unordered_map<string, DataArsip> file_map_nama;
long long int counter = 1;
string nama_folder = "arsip_digital";

string cek_tanggal(string file_path) {
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

void insert_data(string nama_file, string isi, string folder) {
    string mkdir_cmd = "mkdir " + folder + " 2> nul";
    system(mkdir_cmd.c_str());

    ofstream Data(folder + "/" + nama_file);
    if (Data.is_open()) {
        Data << isi;
        Data.close();
        cout << "[System] File fisik berhasil dibuat!\n";
    } else {
        cout << "\nError membuat file\n";
    }
}

// =========================================================
// FUNGSI MEMBACA DATA DARI HARD DRIVE KE RAM (HASH MAP)
// =========================================================
void load_data_ke_memori() {
    // Bersihkan memori lama
    file_map_ukuran.clear();
    file_map_nama.clear();
    counter = 1;

    string command = "dir " + nama_folder + " /b > file_list.txt 2> nul";
    system(command.c_str());

    ifstream file_list("file_list.txt");
    string nama_file;    

    if (file_list.is_open()) {
        while(getline(file_list, nama_file)) {
            if (!nama_file.empty() && nama_file.back() == '\r') {
                nama_file.pop_back();
            }

            string file_path = nama_folder + "/" + nama_file;
            ifstream file(file_path, ios::binary | ios::ate);

            if (file.is_open()) {
                long long int ukuran_file = file.tellg();
                file.clear();
                file.seekg(0, ios::beg);
                string isi_konten((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
                string tanggal = cek_tanggal(file_path);

                file_map_ukuran[ukuran_file].push_back(nama_file);

                DataArsip data_arsip;
                data_arsip.id = "D" + to_string(counter);
                data_arsip.namaFile = nama_file;
                data_arsip.ukuran = ukuran_file;
                data_arsip.tanggal = tanggal;
                data_arsip.sumber = nama_folder;
                data_arsip.konten = isi_konten;

                file_map_nama[nama_file] = data_arsip;

                counter++;
                file.close();
            }
        }
        file_list.close();
    }
    cout << "[System] Berhasil memuat " << file_map_nama.size() << " file ke dalam memori RAM.\n";
}

// =========================================================
// GENERATE DATA DENGAN PELUANG ACAK (RANDOM CHANCE DUPLICATE)
// =========================================================
void populate_data(string folder, int jumlah_data) {
    string del_cmd = "del /q " + folder + "\\* 2> nul";
    system(del_cmd.c_str());
    string mkdir_cmd = "mkdir " + folder + " 2> nul";
    system(mkdir_cmd.c_str());

    cout << "\n[System] Sedang men-generate " << jumlah_data << " file dummy (Ukuran 100KB - 300KB)...\n";
    if(jumlah_data >= 1000) cout << "[System] Proses ini memakan waktu beberapa detik. Mohon tunggu...\n";
    
    srand(time(0));

    string chunk = "DATA_ARSIP_DUMMY_";
    while (chunk.length() < 10240) { // Chunk 10KB
        chunk += "TEKS_ACAK_UNTUK_MEMENUHI_UKURAN_FILE_ARSIP_DIGITAL_";
    }

    int jumlah_duplikat = 0;

    for (int i = 1; i <= jumlah_data; i++) {
        string nama_file = "arsip_data_" + to_string(i) + ".txt";
        string file_path = folder + "/" + nama_file;
        
        // PELUANG 10% UNTUK TERJADI DUPLIKASI DATA
        int chance = rand() % 100; // Angka acak 0 - 99
        
        if (i > 1 && chance < 10) { // Jika masuk 10% dan bukan file pertama
            // Pilih satu file yang sudah pernah dibuat secara acak untuk di-copy
            int target_copy = (rand() % (i - 1)) + 1;
            string source_path = folder + "/arsip_data_" + to_string(target_copy) + ".txt";
            
            ifstream src(source_path, ios::binary);
            ofstream dst(file_path, ios::binary);
            if (src.is_open() && dst.is_open()) {
                dst << src.rdbuf(); // Copy isinya 100% sama
                jumlah_duplikat++;
            }
            src.close();
            dst.close();
        } 
        else {
            // BUAT FILE BARU YANG UNIK (90% Peluang)
            int target_size = 102400 + (rand() % 204801); // 100KB - 300KB
            ofstream file(file_path, ios::binary);
            if (file.is_open()) {
                int current_size = 0;
                while (current_size < target_size) {
                    int remaining = target_size - current_size;
                    if (remaining < chunk.length()) {
                        file << chunk.substr(0, remaining);
                        current_size += remaining;
                    } else {
                        file << chunk;
                        current_size += chunk.length();
                    }
                }
                // Tambahkan ID Unik agar tidak kebetulan sama
                file << "ID_UNIK_" << i << "_" << rand(); 
                file.close();
            }
        }

        if (i % 500 == 0) cout << " -> Berhasil memproses " << i << " file...\n";
    }

    cout << "\n[System] Selesai! Dari " << jumlah_data << " file yang digenerate,\n";
    cout << "         terdapat " << jumlah_duplikat << " data DUPLIKAT yang tercipta secara acak.\n";
}

int main() {
    // Bikin folder jika belum ada
    string mkdir_cmd = "mkdir " + nama_folder + " 2> nul";
    system(mkdir_cmd.c_str());

    // Otomatis memuat data saat program dijalankan
    load_data_ke_memori();

    string menu_text = "\n=== User Menu ===\n"
                       "1. Insert Data Baru\n"
                       "2. Deteksi Duplikasi (Ambil Data Eksperimen)\n"
                       "3. Generate Dummy Data (100 / 1000 / 5000)\n"
                       "-9. Close\n"
                       "Select : ";

    int menu_input = 0;
    while (menu_input != -9) {
        cout << menu_text;
        if (!(cin >> menu_input)) {
            cin.clear(); cin.ignore(1000, '\n'); continue;
        }

        switch(menu_input) {
            case 1: {
                string input_nama_file, isi, input_folder;
                cout << "Input nama file: "; cin >> input_nama_file;
                cout << "Input isi file: "; getline(cin >> ws, isi);
                cout << "Input nama folder: "; cin >> input_folder;

                insert_data(input_nama_file, isi, input_folder);
                
                file_map_ukuran[isi.length()].push_back(input_nama_file);
                DataArsip data_baru;
                data_baru.id = "D" + to_string(counter++);
                data_baru.namaFile = input_nama_file;
                data_baru.ukuran = isi.length();
                data_baru.tanggal = cek_tanggal(input_folder + "/" + input_nama_file);
                data_baru.sumber = input_folder;
                data_baru.konten = isi;
                file_map_nama[input_nama_file] = data_baru;
                break;
            }
            case 2: {
                cout << "\n=== STARTING DUPE DETECTION ===\n";
                if (file_map_nama.empty()) {
                    cout << "[!] Memori kosong! Silakan Generate Data (Menu 3) terlebih dahulu.\n";
                    break;
                }

                auto start_time = chrono::high_resolution_clock::now();
                bool ditemukan = false;
                
                // Tahap 1: Filter Ukuran
                for (auto i : file_map_ukuran) {
                    if (i.second.size() > 1) { 
                        // Tahap 2: Cek Konten
                        unordered_map<string, vector<string>> map_konten_lokal;
                        for (auto nama : i.second) {
                            string konten_file = file_map_nama[nama].konten;
                            map_konten_lokal[konten_file].push_back(nama);
                        }

                        for (auto j : map_konten_lokal) {
                            if (j.second.size() > 1) {
                                ditemukan = true;
                                cout << "-> DUPLIKAT (Ukuran: " << i.first << " Bytes) | File: ";
                                for (auto f : j.second) cout << f << " ";
                                cout << "\n";
                            }
                        }
                    }
                }
                
                auto end_time = chrono::high_resolution_clock::now();
                chrono::duration<double, milli> durasi = end_time - start_time;

                if (!ditemukan) cout << "Tidak ada duplikasi yang ditemukan.\n";
                
                cout << "------------------------------------------\n";
                cout << "[!] WAKTU EKSEKUSI PENCARIAN : " << durasi.count() << " ms\n";
                cout << "------------------------------------------\n";
                break;
            }
            case 3: {
                cout << "\nJumlah data: (1) 100  (2) 1000  (3) 5000 -> Pilih: ";
                int pilihan; cin >> pilihan;

                int jumlah = 100;
                if (pilihan == 2) jumlah = 1000;
                else if (pilihan == 3) jumlah = 5000;

                populate_data(nama_folder, jumlah);
                
                // SEKARANG OTOMATIS LOAD KE MEMORI (TIDAK PERLU RESTART LAGI)
                cout << "[System] Memperbarui Hash Map di RAM...\n";
                load_data_ke_memori();
                break;
            }
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