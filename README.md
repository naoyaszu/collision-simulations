# Collision Simulation

## Deskripsi Proyek

Proyek ini merupakan simulasi tabrakan bola 2D menggunakan bahasa **C++** dan library **SFML**. Setiap bola bergerak secara bebas di dalam window, memantul ketika bertabrakan dengan bola lain maupun dengan dinding. Tujuan utama simulasi ini adalah untuk membandingkan dua metode *collision detection*, yaitu **Brute Force** dan **Quadtree**.

---

## Teknologi yang Digunakan

* **Bahasa** : C++
* **Library Grafis** : SFML (Simple and Fast Multimedia Library)
* **Environment** : WSL (Windows Subsystem for Linux)
* **Compiler** : g++

---

## Konsep Dasar

### 1. Brute Force Collision Detection

Metode brute force bekerja dengan cara **mengecek semua pasangan bola** satu per satu untuk menentukan apakah terjadi tabrakan.

Jika terdapat (n) bola, maka jumlah pengecekan tabrakan adalah:

$\frac{n(n-1)}{2}$

Metode ini cocok untuk simulasi dengan jumlah objek yang sedikit.

---

### 2. Quadtree Collision Detection

Quadtree adalah struktur data pohon yang membagi ruang 2D menjadi empat bagian secara rekursif. Setiap node quadtree menyimpan objek (bola) yang berada dalam wilayah tersebut. Pada metode ini, bola dimasukkan ke dalam quadtree berdasarkan posisinya. Untuk setiap bola, pencarian tabrakan hanya dilakukan pada bola-bola di area sekitarnya. Metode efisien untuk simulasi dengan jumlah objek yang banyak.
Kompleksitas metode ini mendekati $O(n \log n)$.

---

## Struktur Program

### Komponen Utama

1. **Ball**\
   Menyimpan data bola seperti posisi, kecepatan, radius, massa, dan warna.

2. **AABB (Axis-Aligned Bounding Box)**\
   Digunakan sebagai batas wilayah pada quadtree dan untuk proses pencarian kandidat tabrakan.

3. **Quadtree**\
   Struktur data untuk menyimpan bola berdasarkan pembagian ruang.

4. **resolveCollision()**\
   Fungsi *narrow phase* yang menangani fisika pantulan bola secara elastis.

5. **main()**\
   Mengatur inisialisasi bola, loop simulasi, pemilihan metode collision detection, dan rendering.

---

## Kontrol Program

* **Q** : Mengganti metode collision detection (Brute Force / Quadtree)
* **Space** : Pause / Resume simulasi
* **ESC** : Keluar dari program

---

## Cara Kompilasi

Pastikan SFML sudah terinstal, kemudian jalankan perintah berikut:

```bash
g++ collision.cpp -o collision \
    -lsfml-graphics -lsfml-window -lsfml-system
```

Jalankan program:

```bash
./collision
```
---

## Tampilan

1. Brute Force
 ![brute force](images/bruteforce.png)

2. Quad Tree 
 ![quadtree](images/quadtree.png)


https://github.com/user-attachments/assets/241438e1-c08c-41e6-bf54-bceea1ea3c12

