#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <limits> // cin.ignore için

using namespace std;

// --- RENK KODLARI ---
#define RESET   "\033[0m"
#define RED     "\033[31m"      // [9] Geri ve [0] Çıkış için
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BOLD    "\033[1m"

// --- YAPILAR ---

struct Book {
    int id;
    string title;
    string author;
    bool isBorrowed;
    time_t dueDate;
};

struct User {
    string username;
    string password;
};

// --- YARDIMCI FONKSİYONLAR ---

void clearScreen() {
    system("cls"); // Linux/Mac için "clear"
}

void printLogo() {
    cout << MAGENTA << BOLD;
    cout << R"(
  __  __  _   _  ____      _  _____   _      ___ ____  ____      _    ____  __   __
 |  \/  || | | ||  _ \    / \|_   _| | |    |_ _| __ )|  _ \    / \  |  _ \ \ \ / /
 | |\/| || | | || |_) |  / _ \ | |   | |     | ||  _ \| |_) |  / _ \ | |_) | \ V / 
 | |  | || |_| ||  _ <  / ___ \| |   | |___  | || |_) |  _ <  / ___ \|  _ <   | |  
 |_|  |_| \___/ |_| \_\/_/   \_\_|   |_____|___|____/ |_| \_\/_/   \_\_| \_\  |_|  
                                                                                   
    )" << RESET << endl;
}

string formatTime(time_t rawTime) {
    if (rawTime == 0) return "-";
    struct tm* timeinfo;
    char buffer[80];
    timeinfo = localtime(&rawTime);
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M", timeinfo);
    return string(buffer);
}

Book* findBook(vector<Book>& library, int id) {
    for (size_t i = 0; i < library.size(); i++) {
        if (library[i].id == id) return &library[i];
    }
    return nullptr;
}

// --- NAVİGASYON YARDIMCISI ---
// Bekleme yapmak yerine kullanıcıdan işlem bekleyen fonksiyon
void waitOrExit() {
    int choice;
    cout << "\n" << RED << "[9] Geri" << RESET << "    " << RED << "[0] Çıkış" << RESET << endl;
    cout << "Seçiminiz: ";
    cin >> choice;

    if (choice == 0) {
        cout << "Çıkış yapılıyor..." << endl;
        exit(0); // Programı tamamen kapatır
    }
    // 9 veya başka bir tuşsa fonksiyon biter, main'e döner
}

// --- SAYFA FONKSİYONLARI ---

void pageListBooks(const vector<Book>& library) {
    clearScreen();
    printLogo();
    cout << BOLD << ">>> KİTAP ENVANTERİ" << RESET << "\n\n";

    cout << left << setw(5) << "ID" << setw(25) << "Kitap Adı" << setw(20) << "Yazar" << setw(15) << "Durum" << "İade Tarihi" << endl;
    cout << string(90, '-') << endl;

    for (const auto& book : library) {
        cout << left << setw(5) << book.id
            << setw(25) << book.title
            << setw(20) << book.author;

        if (book.isBorrowed) {
            cout << RED << left << setw(15) << "ODUNC ALINDI" << RESET;
            cout << YELLOW << formatTime(book.dueDate) << RESET;
        }
        else {
            cout << GREEN << left << setw(15) << "RAFTAYIM" << RESET;
            cout << "-";
        }
        cout << endl;
    }

    // Sadece listeleme sayfasında seçim yaptırıyoruz
    waitOrExit();
}

void pageBorrowBook(vector<Book>& library) {
    clearScreen();
    printLogo();
    cout << BOLD << ">>> KİTAP ÖDÜNÇ ALMA" << RESET << "\n\n";

    int inputID;
    // İstenilen formatta giriş satırı
    cout << "Ödünç almak istediğiniz Kitap ID (" << RED << "[9] Geri" << RESET << "  " << RED << "[0] Çıkış" << RESET << "): ";
    cin >> inputID;

    if (inputID == 0) exit(0); // Çıkış
    if (inputID == 9) return;  // Geri Dön

    Book* bookPtr = findBook(library, inputID);

    if (bookPtr == nullptr) {
        cout << RED << "\n[!] Hata: Kitap bulunamadı." << RESET << endl;
    }
    else if (bookPtr->isBorrowed) {
        cout << YELLOW << "\n[!] Bu kitap zaten başkasında. İade tarihi: " << formatTime(bookPtr->dueDate) << RESET << endl;
    }
    else {
        time_t now = time(0);
        time_t due = now + (14 * 24 * 60 * 60);

        bookPtr->isBorrowed = true;
        bookPtr->dueDate = due;
        cout << GREEN << "\n[BAŞARILI] '" << bookPtr->title << "' ödünç verildi." << RESET << endl;
        cout << "Son İade Tarihi: " << formatTime(due) << endl;
    }

    // İşlem bitince hemen dönmesin, sonucu görelim
    cout << "\nDevam etmek için bir tuşa basıp Enter yapın...";
    string dummy; cin >> dummy;
}

void pageReturnBook(vector<Book>& library) {
    clearScreen();
    printLogo();
    cout << BOLD << ">>> KİTAP İADE ETME" << RESET << "\n\n";

    int inputID;
    cout << "İade edilecek Kitap ID (" << RED << "[9] Geri" << RESET << "  " << RED << "[0] Çıkış" << RESET << "): ";
    cin >> inputID;

    if (inputID == 0) exit(0);
    if (inputID == 9) return;

    Book* bookPtr = findBook(library, inputID);

    if (bookPtr == nullptr) {
        cout << RED << "\n[!] Hata: Kitap bulunamadı." << RESET << endl;
    }
    else if (!bookPtr->isBorrowed) {
        cout << YELLOW << "\n[!] Bu kitap zaten kütüphanede." << RESET << endl;
    }
    else {
        time_t now = time(0);
        double fine = 0.0;
        double dailyFineRate = 10.0;

        double secondsLate = difftime(now, bookPtr->dueDate);

        if (secondsLate > 0) {
            int daysLate = (int)(secondsLate / (24 * 60 * 60)) + 1;
            fine = daysLate * dailyFineRate;
            cout << RED << BOLD << "\n[!] GECİKME CEZASI HESAPLANDI!" << RESET << endl;
            cout << "Gecikme: " << daysLate << " gün." << endl;
            cout << "Tutar: " << BOLD << fine << " TL" << RESET << endl;
        }
        else {
            cout << GREEN << "\n[BAŞARILI] Zamanında iade için teşekkürler." << RESET << endl;
        }

        bookPtr->isBorrowed = false;
        bookPtr->dueDate = 0;
    }

    cout << "\nDevam etmek için bir tuşa basıp Enter yapın...";
    string dummy; cin >> dummy;
}

// --- GİRİŞ & KAYIT SİSTEMİ ---

void registerUser(vector<User>& users) {
    clearScreen();
    printLogo();
    cout << CYAN << "=== YENİ KULLANICI KAYDI ===" << RESET << "\n\n";

    string u, p;
    cout << "Yeni Kullanıcı Adı: "; cin >> u;
    cout << "Yeni Şifre: "; cin >> p;

    users.push_back({ u, p });

    cout << GREEN << "\n[OK] Kayıt Başarılı!" << RESET << endl;
    cout << "Giriş ekranına yönlendiriliyorsunuz... (Enter'a basın)";
    cin.ignore(); cin.get();
}

bool loginUser(const vector<User>& users) {
    clearScreen();
    printLogo();
    cout << YELLOW << "=== GÜVENLİK GİRİŞİ ===" << RESET << "\n\n";

    string u, p;
    cout << "Kullanıcı Adı: "; cin >> u;
    cout << "Şifre: "; cin >> p;

    for (const auto& user : users) {
        if (user.username == u && user.password == p) {
            cout << GREEN << "\n[✓] Giriş Onaylandı. Hoşgeldin " << u << "!" << RESET << endl;
            cin.ignore(); cin.get();
            return true;
        }
    }

    cout << RED << "\n[X] Hatalı kullanıcı adı veya şifre!" << RESET << endl;
    cout << "Tekrar denemek için Enter'a basın...";
    cin.ignore(); cin.get();
    return false;
}

bool authenticationSystem(vector<User>& users) {
    while (true) {
        clearScreen();
        printLogo();
        cout << BOLD << "   [ GİRİŞ EKRANI ]" << RESET << endl;
        cout << "   1. Giriş Yap" << endl;
        cout << "   2. Kayıt Ol" << endl;
        cout << "\n   " << RED << "[0] Çıkış" << RESET << endl;
        cout << "\n   Seçiminiz: ";

        int choice;
        cin >> choice;

        if (choice == 0) return false;

        if (choice == 1) {
            if (loginUser(users)) return true;
        }
        else if (choice == 2) {
            registerUser(users);
        }
        else {
            cout << RED << "Geçersiz seçim!" << RESET;
        }
    }
}

// --- MAIN ---

int main() {
    setlocale(LC_ALL, "Turkish");

    vector<User> systemUsers = { {"admin", "1234"} };

    // Giriş Sistemi
    if (!authenticationSystem(systemUsers)) {
        return 0; // Girişte çıkışa basıldıysa
    }

    // Kütüphane Verisi
    vector<Book> myLibrary = {
        {101, "Nutuk", "M.K. Ataturk", false, 0},
        {102, "Sefiller", "Victor Hugo", false, 0},
        {103, "Suc ve Ceza", "Dostoyevski", false, 0},
        {104, "Harry Potter", "J.K. Rowling", false, 0}
    };

    int choice;

    // ANA MENÜ DÖNGÜSÜ
    while (true) {
        clearScreen();
        printLogo();
        cout << BOLD << "   [ ANA MENÜ ]" << RESET << endl;
        cout << "   1. Kitapları Listele" << endl;
        cout << "   2. Kitap Ödünç Al" << endl;
        cout << "   3. Kitap İade Et" << endl;
        cout << "\n   " << RED << "[0] Çıkış" << RESET << endl;

        cout << "\n   Seçiminiz: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
        case 1: pageListBooks(myLibrary); break;
        case 2: pageBorrowBook(myLibrary); break;
        case 3: pageReturnBook(myLibrary); break;
        case 0: exit(0); // Direkt çıkış
        default: break;
        }
    }

    return 0;
}