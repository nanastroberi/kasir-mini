#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <sstream>
#include <tuple>
#include <numeric>
#include <algorithm>
#include <limits>

using namespace std;

// Model data simpel: hanya ID, Nama, dan Harga
struct Barang {
    int id;
    string nama;
    int harga;

    // Representasi string bergaya segmen HL7: ID|Nama|Harga
    string toHL7() const {
        return to_string(id) + "|" + nama + "|" + to_string(harga);
    }
};

// Class khusus untuk manajemen data dan file txt
class KasirManager {
private:
    list<Barang> daftar;
    string namaFile;
    int nextId = 101;

    bool parseDanTambahHL7(const string& record) {
        stringstream ss(record);
        string sId, nama, sHarga;

        if (getline(ss, sId, '|') && getline(ss, nama, '|') && getline(ss, sHarga, '|')) {
            try {
                int id = stoi(sId);
                int harga = stoi(sHarga);
                daftar.push_back({ id, nama, harga });
                if (id >= nextId) nextId = id + 1;
                return true;
            }
            catch (...) {
                return false;
            }
        }
        return false;
    }

    // Simpan seluruh daftar ke file .txt dalam format ID|Nama|Harga
    void simpanKeFile() const {
        ofstream file(namaFile, ios::trunc);
        if (!file.is_open()) {
            cout << "Gagal menyimpan ke file " << namaFile << "\n";
            return;
        }
        for (const auto& item : daftar) {
            file << item.toHL7() << "\n";
        }
    }

    // Muat data dari file .txt (jika belum ada, buat 5 barang awal)
    void muatDariFile() {
        ifstream file(namaFile);
        if (!file.is_open()) {
            daftar = {
                { 101, "Kopi Susu",    18000 },
                { 102, "Roti Bakar",   25000 },
                { 103, "Air Mineral",   6000 },
                { 104, "Nasi Goreng",  32000 },
                { 105, "Es Teh Manis",  8000 }
            };
            nextId = 106;
            simpanKeFile();
            return;
        }

        string baris;
        while (getline(file, baris)) {
            if (!baris.empty()) {
                parseDanTambahHL7(baris);
            }
        }
    }

public:
    KasirManager(const string& file = "daftar_barang.txt") : namaFile(file) {
        muatDariFile();
    }

    // Insert data baru (ID otomatis + langsung simpan ke file)
    int insertData(const string& nama, int harga) {
        int idBaru = nextId++;
        daftar.push_back({ idBaru, nama, harga });
        simpanKeFile();
        return idBaru;
    }

    // Hapus data berdasarkan ID (+ langsung update file)
    bool removeById(int id) {
        size_t jumlahAwal = daftar.size();
        daftar.remove_if([id](const Barang& b) {
            return b.id == id;
            });

        if (daftar.size() < jumlahAwal) {
            simpanKeFile();
            return true;
        }
        return false;
    }

    // Tampilkan semua data dalam format HL7
    void tampilkan() const {
        cout << "\nID|Nama|Harga\n";
        cout << "------------------------\n";
        if (daftar.empty()) {
            cout << "(Data kosong)\n";
            return;
        }
        for (const auto& item : daftar) {
            cout << item.toHL7() << "\n";
        }
    }

    // Hitung total, termahal, dan termurah lalu kembalikan sebagai tuple
    tuple<int, Barang, Barang> getRingkasan() const {
        if (daftar.empty()) {
            Barang kosong = { 0, "-", 0 };
            return make_tuple(0, kosong, kosong);
        }

        int total = accumulate(daftar.begin(), daftar.end(), 0,
            [](int sum, const Barang& b) {
                return sum + b.harga;
            });

        auto bandingHarga = [](const Barang& a, const Barang& b) {
            return a.harga < b.harga;
            };

        Barang termahal = *max_element(daftar.begin(), daftar.end(), bandingHarga);
        Barang termurah = *min_element(daftar.begin(), daftar.end(), bandingHarga);

        return make_tuple(total, termahal, termurah);
    }
};

int bacaInt(const string& prompt) {
    int nilai;
    while (true) {
        cout << prompt;
        if (cin >> nilai) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return nilai;
        }
        cout << "Input harus berupa angka. Coba lagi.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void tampilkanMenu() {
    cout << "\n=== KASIR MINI ===\n"
        << "1. Lihat Daftar Barang\n"
        << "2. Tambah Barang\n"
        << "3. Hapus Barang by ID\n"
        << "4. Lihat Ringkasan Belanja\n"
        << "0. Keluar\n";
}

int main() {
    KasirManager kasir("daftar_barang.txt");

    int pilihan;
    do {
        tampilkanMenu();
        pilihan = bacaInt("Pilih menu: ");

        switch (pilihan) {
        case 1:
            kasir.tampilkan();
            break;

        case 2: {
            string nama;
            cout << "Nama barang : ";
            getline(cin, nama);

            replace(nama.begin(), nama.end(), '|', '-');

            int harga = bacaInt("Harga       : ");
            int idBaru = kasir.insertData(nama, harga);
            cout << "Berhasil ditambah dengan ID: " << idBaru << "\n";
            break;
        }

        case 3: {
            int idHapus = bacaInt("Masukkan ID yang ingin dihapus: ");
            if (kasir.removeById(idHapus)) {
                cout << "Barang ID " << idHapus << " berhasil dihapus.\n";
            }
            else {
                cout << "Barang ID " << idHapus << " tidak ditemukan.\n";
            }
            break;
        }

        case 4: {
            auto [totalBelanja, barangTermahal, barangTermurah] = kasir.getRingkasan();

            cout << "\n=== RINGKASAN BELANJA ===\n";
            cout << "Total Belanja   : Rp " << totalBelanja << "\n";
            cout << "Barang Termahal : " << barangTermahal.toHL7() << "\n";
            cout << "Barang Termurah : " << barangTermurah.toHL7() << "\n";
            break;
        }

        case 0:
            cout << "Program selesai.\n";
            break;

        default:
            cout << "Pilihan tidak valid.\n";
            break;
        }
    } while (pilihan != 0);

    return 0;
}