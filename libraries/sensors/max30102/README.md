# Dokumentasi API MAX30102

Library ini menyediakan antarmuka (interface) untuk menggunakan sensor Heart Rate & SpO2 MAX30102 pada board FPGA Nusacore yang berjalan dengan arsitektur PULP (Parallel Ultra Low Power) runtime. Library ini dirancang agar mudah digunakan untuk mengakses data dari sensor, mengonfigurasi pengaturan dasar, serta memonitor status perangkat.

---

## 1. Struktur Library
- **`max30102.h`**: File header utama yang berisi deklarasi fungsi dan struktur data. File ini yang harus di-_include_ pada aplikasi utama Anda.
- **`max30102_regs.h`**: File header yang berisi definisi register internal dari MAX30102 (alamat-alamat register).
- **`max30102.c`**: File sumber (source code) yang berisi implementasi logika driver untuk berkomunikasi dengan sensor menggunakan antarmuka I2C bawaan dari PULP runtime.

---

## 2. Struktur Data dan Enumerasi

### `max30102_status_t`
Tipe data enumerasi yang digunakan sebagai nilai kembalian (return value) untuk setiap fungsi dalam library guna mengetahui status eksekusi.
- `MAX30102_OK = 0`: Eksekusi berhasil tanpa error.
- `MAX30102_ERROR_I2C = -1`: Terjadi kegagalan komunikasi I2C (gagal membaca/menulis ke sensor).
- `MAX30102_ERROR_NOT_FOUND = -2`: Sensor tidak terdeteksi (Part ID tidak sesuai, mungkin sensor belum terhubung).
- `MAX30102_ERROR_INVALID_PARAM = -3`: Parameter yang diberikan tidak valid (misal: memberikan pointer *null*).

### `max30102_t`
Struktur data (*context struct*) yang merepresentasikan sebuah instance dari sensor MAX30102. Struktur ini menyimpan informasi koneksi ke hardware.
```c
typedef struct {
    int i2c_port;     // Port I2C hardware yang digunakan
    uint8_t i2c_addr; // Alamat I2C sensor (secara default adalah 0x57)
    void* i2c_dev;    // Pointer ke struktur device I2C internal bawaan PULP runtime
} max30102_t;
```

---

## 3. Referensi Fungsi (API)

### Inisialisasi dan Konfigurasi Dasar

#### `max30102_init`
```c
max30102_status_t max30102_init(max30102_t *dev, int i2c_port);
```
Fungsi ini digunakan untuk melakukan inisialisasi sensor. Langkah-langkah yang dilakukan secara internal meliputi:
1. Memeriksa sambungan ke sensor dengan membaca Part ID.
2. Melakukan *soft reset* untuk mengembalikan state sensor.
3. Mengatur parameter ke nilai *default* yang optimal, yaitu:
   - Mode operasi: SpO2.
   - Sample Rate: 400 sample per detik (sps).
   - Resolusi ADC: 18-bit.
   - Averaging (rata-rata) pada FIFO: 4 sampel.
4. Mengaktifkan fitur interrupt sensor.

- **Parameter**: 
  - `dev`: Pointer ke variabel instance `max30102_t`.
  - `i2c_port`: Nomor Port I2C yang digunakan pada perangkat keras (umumnya `0`).
- **Return**: `MAX30102_OK` jika berhasil diinisialisasi.

#### `max30102_reset`
```c
max30102_status_t max30102_reset(max30102_t *dev);
```
Memerintahkan sensor untuk melakukan *soft reset*. Fungsi ini secara internal akan me-reset *pointer FIFO* dan semua nilai register ke *state* awal bawaan pabrik (power-on).

#### `max30102_get_part_id`
```c
max30102_status_t max30102_get_part_id(max30102_t *dev, uint8_t *part_id);
```
Membaca Register Part ID (Identitas Perangkat) bawaan dari hardware sensor. Sensor MAX30102 yang normal akan mengembalikan nilai `0x15`. Fungsi ini sangat berguna untuk memverifikasi jalur komunikasi I2C sebelum melakukan perintah lain.

#### `max30102_set_led_amplitude`
```c
max30102_status_t max30102_set_led_amplitude(max30102_t *dev, uint8_t red_pa, uint8_t ir_pa);
```
Digunakan untuk kalibrasi kecerahan LED. Menetapkan seberapa besar arus LED Merah (Red) dan Infra-merah (IR) yang akan memancarkan cahaya ke jaringan kulit.
- Rentang nilai yang diizinkan adalah `0x00` (Mati) hingga `0xFF` (~50mA). 
- Nilai *default* saat inisialisasi adalah `0x24` (~7mA).
- Pengaturan arus ini penting agar ADC tidak jenuh/saturasi jika jaringan kulit yang diukur tipis.

### Pengambilan Data (FIFO)

#### `max30102_clear_fifo`
```c
max30102_status_t max30102_clear_fifo(max30102_t *dev);
```
Mereset nilai pointer `Write`, `Read`, dan `Overflow` pada FIFO internal sensor kembali ke `0`. Sebaiknya dipanggil sebelum memulai pengambilan data konstan agar membuang data lama atau *sampah*.

#### `max30102_read_fifo`
```c
max30102_status_t max30102_read_fifo(max30102_t *dev, uint32_t *red_data, uint32_t *ir_data);
```
Mengambil 1 blok data *sample* dari FIFO sensor. Setiap blok terdiri dari data pembacaan LED Merah (Red) dan LED Infra-Merah (IR).
- **Parameter**: 
  - `dev`: Pointer ke instance `max30102_t`.
  - `red_data`: Pointer ke variabel tujuan untuk menyimpan data Merah (format 32-bit).
  - `ir_data`: Pointer ke variabel tujuan untuk menyimpan data Infra-Merah (format 32-bit).
- **Return**: `MAX30102_OK` bila eksekusi bacaan selesai. Jika tidak ada data baru pada antrean (Pointer Write = Pointer Read), maka fungsi akan mengembalikan status sukses tetapi nilai pada `red_data` dan `ir_data` **tidak berubah**.

---

## 4. Cara Penggunaan Test File (`test_max30102.c`)

File test (`test_max30102.c`) dibuat untuk menguji fungsionalitas dari fungsi-fungsi library di atas. Tergantung lingkungan (*environment*), terdapat dua cara untuk menjalankan/build test file ini.

### Cara 1: Kompilasi dan Build untuk Arsitektur PULP / FPGA (Rekomendasi)
Cara ini dilakukan jika ingin menguji pada board FPGA Nusacore sebenarnya atau menggunakan simulator PULP. Anda akan membutuhkan file `Makefile` yang telah dikonfigurasikan dengan PULP SDK di direktori yang sama.

1. **Persiapkan Environment:** Buka terminal dan aktifkan environment untuk memuat PULP GCC toolchain (wajib dilakukan setiap kali membuka terminal baru).
   ```bash
   source ./setup_env.sh
   ```
2. **Membersihkan File Build Lama:** Menghapus artefak hasil kompilasi dari percobaan sebelumnya untuk menghindari konflik build.
   ```bash
   make clean
   ```
3. **Kompilasi Program (Build):**
   ```bash
   make all
   ```
   *Catatan:* Proses ini akan mengkompilasi sumber daya program (`test_max30102.c` beserta dependensinya) menjadi file `.o` (object). Hasil build akan secara otomatis disimpan di dalam direktori `build/`.
4. **Jalankan Aplikasi:** Jika Anda ingin mensimulasikan hasil eksekusinya menggunakan PULP Runner atau menjalankannya di FPGA.
   ```bash
   make run
   ```

### Cara 2: Software Test / Mock Test Secara Lokal (PC)
Jika ingin memastikan logika program atau mengecek syntax pada PC (secara dummy), bisa mengkompilasinya menggunakan kompiler GCC lokal PC, dengan mengaktifkan flag `SOFTWARE_TEST` untuk mengabaikan fungsi I2C perangkat keras secara nyata.

Buka terminal dan jalankan:
```bash
gcc test_max30102.c max30102.c -I. -DSOFTWARE_TEST -o test_soft
```
Jalankan file yang telah terbentuk:
```bash
./test_soft
```

---

## 5. Contoh Penggunaan Dasar dalam Program
Di bawah ini merupakan skema aliran dasar *loop* aplikasi utama dalam memanfaatkan library MAX30102.
```c
#include "max30102.h"
#include <stdio.h>

int main() {
    max30102_t sensor;
    
    // Inisialisasi sensor dan menggunakan interface I2C_0
    if (max30102_init(&sensor, 0) != MAX30102_OK) {
        printf("Gagal menginisialisasi MAX30102\n");
        return -1;
    }
    printf("MAX30102 Berhasil Diinisialisasi!\n");
    
    // Looping utama aplikasi untuk mengambil data secara berkelanjutan
    uint32_t red_val, ir_val;
    while(1) {
        // Ambil data FIFO jika ada
        if(max30102_read_fifo(&sensor, &red_val, &ir_val) == MAX30102_OK) {
             printf("Data Red: %u, Data IR: %u\n", red_val, ir_val);
             
             // TODO: Proses data raw ke algoritma pengukuran detak jantung & oksigen
        }
    }
    return 0;
}
```
