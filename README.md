# Tucil3_13524020_13524031

## Deskripsi Singkat
Program ini adalah solver Ice Sliding Puzzle berbasis C++ yang menerapkan UCS, GBFS, dan A* untuk mencari lintasan optimal berdasarkan biaya tile. State mencakup posisi aktor dan progres checkpoint sehingga aturan urutan digit, lava, dan mekanisme sliding dapat dipenuhi. Hasil pencarian dilengkapi metrik seperti total cost, jumlah iterasi, dan waktu eksekusi, serta jejak langkah agar solusi mudah diverifikasi. Program dapat dijalankan via CLI maupun GUI (raylib) untuk visualisasi langkah per langkah.

## Requirement
- C++17 compiler (mis. g++)
- Make
- raylib (untuk GUI)

## Kompilasi
Gunakan Makefile yang sudah tersedia:
```bash
make
```

Jika ingin membangun tanpa GUI, pastikan target yang relevan pada Makefile tersedia.

## Menjalankan dan Menggunakan Program
Setelah kompilasi, jalankan executable dari folder `bin/`:
```bash
./bin/solver
```

Secara umum, alur penggunaan:
1. Pilih mode CLI atau GUI.
2. Pilih file input dari folder `test/`.
3. Pilih algoritma (UCS, GBFS, A*), serta heuristik jika diminta.
4. Jalankan solver dan tinjau hasil.

### Alur Penggunaan GUI
1. Pilih file input dari daftar pada folder `test/`.
2. Pilih algoritma (UCS, GBFS, A*).
3. Jika memilih GBFS atau A*, pilih heuristik yang tersedia.
4. Tekan tombol **Solve** untuk menjalankan pencarian.
5. Setelah solusi ditemukan, panel ringkasan menampilkan cost, iterasi, dan waktu.
6. Gunakan kontrol playback (play/pause, slider, atau step) untuk menelusuri setiap langkah.
7. Tekan tombol **Save** untuk menyimpan hasil solusi.

### Folder Save Default
Secara default, hasil solusi disimpan sebagai file teks di folder `test/solution/`. Nama berkas mengikuti pola `solution_<nama_input>.txt`.

## Author
- Stevanus Agustaf Wongso (13524020)
- Vincent Rionarlie (13524031)