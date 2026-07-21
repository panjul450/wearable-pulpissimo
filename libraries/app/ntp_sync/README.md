# NTP Sync - Modul Sinkronisasi Waktu Jaringan untuk RTC DS1307

Library untuk mengambil waktu akurat dari internet (server NTP) dan menyinkronkannya ke modul RTC DS1307 pada platform PULPissimo.

---

## Fitur

| Fitur | Deskripsi |
|-------|-----------|
| Sync 1x | Panggil `ntp_sync_now()` untuk ambil waktu dari internet dan set ke RTC |
| Set Manual | Panggil `ntp_manual_set()` untuk set waktu tanpa internet |
| Dual Mode | Mendukung Wi-Fi Laptop (testing) dan Ethernet Board (deployment) |
| Error Handling | Kode error spesifik (`-1` s/d `-7`) untuk debugging |

---

## Cara Compile dan Jalankan

### Mode Laptop (Testing via Wi-Fi)
```bash
cd libraries/app/ntp_sync
make
./test_ntp
```

### Mode Board (PULPissimo via Ethernet)
```bash
cd libraries/app/ntp_sync
source build.sh
```

---

## API Reference

### `int ntp_sync_now(i2c_t *rtc_handle)`
Mengambil waktu dari internet (NTP) dan langsung menyimpannya ke RTC. Dipanggil 1 kali saja.

| Parameter | Deskripsi |
|-----------|-----------|
| `rtc_handle` | Handle I2C yang diperoleh dari `rtc_init()` |

| Return | Keterangan |
|--------|------------|
| `0` (NTP_OK) | Sukses, waktu RTC sudah ter-update |
| `-1` s/d `-7` | Error (lihat tabel di bawah) |

**Contoh:**
```c
i2c_t *rtc_handle;
i2c_dev_t dev;
rtc_init(&dev, &rtc_handle);

int rc = ntp_sync_now(rtc_handle);
if (rc == NTP_OK) {
    printf("Jam sudah sinkron dengan internet!\n");
}
```

---

### `int ntp_manual_set(i2c_t *rtc_handle, rtc_time_t *waktu)`
Set waktu RTC secara manual tanpa internet.

**Contoh:**
```c
rtc_time_t waktu = {
    .hours = 14, .minutes = 30, .seconds = 0,
    .date = 14, .month = 7, .year = 26, .day = 1
};
ntp_manual_set(rtc_handle, &waktu);
```

---

## Error Codes

| Kode | Konstanta | Sumber | Artinya |
|------|-----------|--------|---------|
| `0` | `NTP_OK` | - | Sukses |
| `-1` | `NTP_ERR_SOCKET` | `ntp_network_connect()` | Gagal buka socket |
| `-2` | `NTP_ERR_DNS` | `ntp_network_connect()` | Gagal resolve DNS |
| `-3` | `NTP_ERR_CONNECT` | `ntp_network_connect()` | Gagal koneksi ke server |
| `-4` | `NTP_ERR_SEND` | `ntp_fetch_timestamp()` | Gagal kirim paket |
| `-5` | `NTP_ERR_TIMEOUT` | `ntp_fetch_timestamp()` | Timeout 5 detik |
| `-6` | `NTP_ERR_RTC_SET` | `time_sync_to_rtc()` | Gagal set RTC (I2C) |
| `-7` | `NTP_ERR_INVALID` | `time_sync_to_rtc()` | Data waktu tidak valid |

---

## Cara Mengubah Mode Laptop → Board (OTOMATIS!)

Kodingan ini **sudah 100% otomatis**! Anda **TIDAK PERLU menghapus atau mengubah apapun** di kodingan saat pindah dari Laptop ke Board.

Semuanya diatur otomatis dari cara Anda melakukan compile:
1. **Mode Laptop:** Saat Anda mengetik `gcc ... -DTEST_DI_LAPTOP`, saklar akan otomatis mengaktifkan library Wi-Fi Linux.
2. **Mode Board:** Saat Anda menjalankan `source ./build.sh`, saklar akan otomatis mengaktifkan library Ethernet PULPissimo (karena tidak ada flag `-DTEST_DI_LAPTOP`).

Tugas tim selanjutnya hanyalah mengisi bagian `#else` di Fungsi 1 dan 2 dengan library Ethernet bawaan board (LwIP). Fungsi 3 tidak perlu diubah sama sekali.
