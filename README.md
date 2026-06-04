# 🔍 Sistem Deteksi Duplikasi Data — Kelompok 7

Proyek ini adalah implementasi program C++ berbasis CLI (Command Line Interface) untuk mensimulasikan sistem deteksi duplikasi arsip atau file. Sistem ini dirancang secara khusus untuk membandingkan performa dari tiga jenis struktur data utama dalam melakukan operasi CRUD (Create, Read, Update, Delete) dan proses pencarian data duplikat.

Tiga struktur data yang diuji dan dibandingkan:
1. **Hash Table** (`std::unordered_map`)
2. **Binary Search Tree (BST) / Ordered Map** (`std::map`)
3. **Vector / Linear Search** (`std::vector`)

## 🌟 Fitur Utama

- **Manajemen Data (CRUD)**: Melakukan penambahan, pencarian, pembaruan, dan penghapusan data arsip. Mendukung pencarian berdasarkan ID, Nama, atau Rentang Ukuran file.
- **Deteksi Duplikasi (Konten & Metadata)**: Mendeteksi keberadaan file ganda menggunakan komparasi isi file secara langsung maupun berdasarkan atribut metadata (nama dan ukuran).
- **Perbandingan Performa secara Real-Time**: Fitur ini memungkinkan Anda membandingkan langsung eksekusi fungsi dari ketiga struktur data tersebut (Hash, BST, Vector).
- **Eksperimen Otomatis & Laporan (*Benchmark*)**: Sistem bisa di-generate ribuan data *dummy* secara otomatis dan menghasilkan _report_ performa berupa file **`performance_report.csv`** dan grafik visual **`grafik_performa.html`** di dalam folder `output/`.

---

## 🚀 Cara Menjalankan Program (Menggunakan CMake)

Proyek ini menggunakan **CMake** sehingga mudah di-*compile* secara modular di berbagai *platform* (Windows, macOS, Linux).

### Prasyarat
Sebelum menjalankan program, pastikan komputer/laptop Anda sudah terinstal:
1. **C++ Compiler** (mendukung standar C++17):
   - **macOS**: `clang++` (dapat diinstal dengan menjalankan `xcode-select --install` di Terminal).
   - **Windows**: MinGW-w64, MSYS2, atau Visual Studio (MSVC).
   - **Linux**: `g++` (bisa diinstal via `sudo apt install build-essential`).
2. **CMake**:
   - **macOS**: Diinstal via Homebrew dengan menjalankan `brew install cmake`.
   - **Windows / Linux**: Bisa di-download di [cmake.org](https://cmake.org/download/) atau via *package manager* bawaan.

### Langkah-langkah untuk macOS / Linux

Buka aplikasi **Terminal**, navigasikan (*cd*) ke dalam *folder* utama proyek ini, lalu jalankan secara berurutan:

1. **Buat folder build dan masuk ke dalamnya:**
   ```bash
   mkdir build
   cd build
   ```

2. **Generate konfigurasi Makefile:**
   ```bash
   cmake ..
   ```

3. **Compile programnya:**
   ```bash
   make
   ```

4. **Jalankan program (Executable):**
   ```bash
   ./deteksi_duplikasi
   ```

### Langkah-langkah untuk Windows

Buka **Command Prompt (CMD)** atau **PowerShell**, arahkan (*cd*) ke *folder* utama proyek ini:

1. **Buat folder build dan masuk ke dalamnya:**
   ```cmd
   mkdir build
   cd build
   ```

2. **Generate konfigurasi project:**
   *(Catatan: pastikan compiler bawaan/MinGW sudah masuk ke PATH Environment Variables)*
   ```cmd
   cmake ..
   ```

3. **Compile programnya:**
   ```cmd
   cmake --build .
   ```

4. **Jalankan programnya:**
   ```cmd
   .\Debug\deteksi_duplikasi.exe
   ```
   *(Tergantung compiler yang dipakai, letak `.exe`-nya bisa langsung di dalam folder `build` sebagai `deteksi_duplikasi.exe`)*

---

## 📂 Struktur Repositori

- `main.cpp` : Entry point aplikasi dan interface Menu CLI.
- `detection.cpp` / `.hpp` : Logika utama algoritma pendeteksian duplikasi file (Hash, BST, Vector).
- `storage.cpp` / `.hpp` : Manajemen indeks memori dan fungsi baca/tulis data arsip ke *storage* fisik.
- `experiment.cpp` / `.hpp` : Modul untuk *benchmarking*, otomatisasi *generate* *dummy file*, serta ekspor laporan CSV/HTML.
- `utils.cpp` / `.hpp` : Kumpulan fungsi utilitas (format penamaan, format bytes, cek tanggal, dll).
- `CMakeLists.txt` : File konfigurasi *build* untuk _compiler_ menggunakan CMake.

---

## 📈 Laporan Output
Setelah menjalankan menu eksperimen (pilihan nomor 12 di menu), sistem akan meng- *output* hasil komparasi memori dan kecepatan proses (dalam *milliseconds*) ke dalam *folder* `output/`:
- **`performance_report.csv`**
- **`grafik_performa.html`** (Bisa dibuka dengan *browser* apa saja untuk melihat grafik batang performa).
