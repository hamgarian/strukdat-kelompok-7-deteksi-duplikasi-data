# Rencana Pembaruan Laporan DOCX Struktur Data (Berdasarkan Hasil Eksperimen 10K)

Dokumen ini berisi panduan instruksional untuk memperbarui **file Word (.DOCX)** Laporan Akhir Anda, berdasarkan data performa nyata (*real-world benchmark*) terbaru di skala 10.000 file.

## 1. Bab III: Desain Sistem & Metodologi
**Fokus: 3.4 Skenario Pengujian**
- **Hapus** narasi lama yang menyebutkan "dataset terdiri atas 500 file arsip digital".
- **Ganti dengan:** Pengujian dilakukan secara dinamis dan berjenjang menggunakan dataset berukuran 1.000, 5.000, dan 10.000 file.
- **Tambahkan:** Eksperimen divariasikan ke dalam 3 tingkat persentase duplikasi (10%, 15%, dan 20%), dan masing-masing skenario direplikasi sebanyak 3 kali agar hasil waktu pengukurannya valid (tidak bias oleh latensi *background process* sistem operasi).

## 2. Bab V: Eksperimen dan Pengujian
**Fokus: 5.1 Skenario Uji**
- **Hapus** studi kasus yang merujuk pada "penemuan 42 grup duplikat pada total 85 file".
- **Ganti dengan:** Penjelasan bahwa sistem diuji melalui modul **Eksperimen Otomatis**, yang secara serentak menguji 27 kali iterasi simulasi komparatif antara struktur data Hash Table, BST, dan Vector.

## 3. Bab VI: Hasil & Analisis (Krusial)
**Fokus: 6.1 Tabel dan Grafik Perbandingan Performa**
- **Update Tabel 6.1:** Masukkan angka komparasi performa di titik puncak (10.000 Data) yang telah terbukti dari `performance_report.csv`:
  - **Hash Table**: Waktu $\approx 20\text{ ms}$, Memori $\approx 23\text{ MB}$
  - **BST**: Waktu $\approx 28\text{ ms}$, Memori $\approx 12\text{ MB}$
  - **Vector**: Waktu $\approx 310\text{ ms}$, Memori $\approx 12,6\text{ MB}$
- **Update Gambar Grafik:** Ganti Gambar 6.1 dan Gambar 6.2 dengan tangkapan layar (*screenshot*) grafik interaktif yang baru saja dihasilkan oleh file `grafik_performa.html`.

**Fokus: 6.2 Analisis Waktu Deteksi Duplikasi**
- **Instruksi Analisis:** Jelaskan mengapa waktu komputasi **Vector** melonjak tajam (hingga lebih dari 300 ms, padahal di skala 500 file hanya 32 ms). Berikan argumen bahwa karena Vector mengeksekusi komparasi secara *Linear Search* $O(n^2)$, performanya akan kolaps bila diterapkan pada *Big Data*.
- Sebutkan bahwa **Hash Table** berhasil menduduki peringkat pertama untuk kecepatan komputasi komparasi string karena kapabilitas pencariannya yang berada di level konstan $O(1)$.

**Fokus: 6.3 Analisis Penggunaan Memori**
- **SANGAT PENTING (Anomali Memori):** Anda wajib menyebutkan secara tertulis mengapa di grafik (pada skala 10.000 file) kurva/batang penggunaan memori justru **menurun** atau lebih kecil dari skala sebelumnya. Hal ini karena di kode program, kita sengaja menerapkan *Dynamic File Sizing* (ukuran rata-rata file diperkecil menjadi ~750 bytes untuk 10k+ data) untuk mencegah *error OOM* (Memory Crash) pada kompilasi 32-bit. Jangan sampai hal ini tidak dijelaskan, karena tanpa penjelasan, grafiknya akan terlihat keliru atau cacat di mata dosen.
- **Instruksi Analisis:** Berikan sorotan tajam bahwa kecepatan konstan Hash Table ternyata memakan *overhead* memori yang sangat boros ($\approx 23$ MB), hampir **dua kali lipat** lebih rakus daripada BST ($\approx 12$ MB).
- Jelaskan secara teori bahwa `std::unordered_map` (Hash Table) harus mengalokasikan banyak memori kosong sebagai *bucket* untuk menjaga rasio tabrakan (*load factor*) serendah mungkin, sementara elemen pada *node* pohon BST dibuat dinamis sesuai data aslinya.

**Fokus: 6.4 Diskusi Trade-off (Kesimpulan Bab VI)**
- **Instruksi Kesimpulan:** Tarik kesimpulan empiris berdasarkan data. Jika prioritas perusahaan arsip digital adalah kecepatan deteksi yang mutlak secepat kilat (mengabaikan mahalnya harga RAM server), maka **Hash Table** adalah juaranya. Namun, jika penyimpanan *cloud/on-premise* perusahaan terbatas, maka **BST** adalah solusi ideal (titik imbang antara memori yang paling kecil dengan kecepatan komputasi yang tinggi).
