# Cara Menjalankan Program (Menggunakan CMake)

Proyek ini menggunakan **CMake** sehingga mudah di-*compile* di berbagai *platform* (Windows, macOS, Linux).

## Prasyarat
Sebelum menjalankan program, pastikan komputer/laptop Anda sudah terinstal:
1. **C++ Compiler**:
   - **macOS**: `clang++` (dapat diinstal dengan menjalankan `xcode-select --install` di Terminal).
   - **Windows**: MinGW-w64, MSYS2, atau Visual Studio (MSVC).
   - **Linux**: `g++` (bisa diinstal via `sudo apt install build-essential`).
2. **CMake**:
   - **macOS**: Diinstal via Homebrew dengan menjalankan `brew install cmake`.
   - **Windows / Linux**: Bisa di-download di [cmake.org](https://cmake.org/download/) atau via *package manager* bawaan.

---

## Langkah-langkah untuk macOS / Linux

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

---

## Langkah-langkah untuk Windows

Buka **Command Prompt (CMD)** atau **PowerShell**, arahkan (*cd*) ke *folder* utama proyek ini:

1. **Buat folder build dan masuk ke dalamnya:**
   ```cmd
   mkdir build
   cd build
   ```

2. **Generate konfigurasi project:**
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
