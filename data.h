#pragma once
// Couche données (SQLite). Modèle : livre, exemplaire, lecteur, pret, attente.
#include <QDate>
#include <QList>
#include <QString>
#include <QStringList>

struct Book {
    int id = 0;
    QString title, author, isbn;
    int copies = 0;    // exemplaires possédés
    int avail = 0;     // exemplaires disponibles (etat = « Disponible »)
    int waiting = 0;   // demandes en liste d'attente (En attente + Prioritaire)
    int prio = 0;      // lecteurs prioritaires (un exemplaire leur est réservé)
    int loans30 = 0;   // prêts sur les 30 derniers jours (popularité)
    QDate added;
    QDate due;         // prochain retour prévu (invalide si aucun prêt en cours)
};

struct Copy {
    int id = 0, bookId = 0;
    QString barcode;
    QString state;     // « Disponible » ou « Emprunté »
    QString borrower;  // lecteur actuel si emprunté
    QDate due;
};

struct Reader {
    int id = 0;
    QString name, matricule, phone;
    int loans = 0;     // prêts en cours
    int late = 0;      // dont en retard
};

struct Loan {
    int id = 0, copyId = 0;
    QString reader, book, barcode;
    QDate loaned, due, returned;
    QString status;    // En cours / En retard / Rendu
};

struct WaitEntry {
    int id = 0, bookId = 0, readerId = 0, position = 0;
    QString reader, book;
    QDate date;
    QString status;    // En attente / Prioritaire
};

struct Stats {
    int books = 0, copies = 0, readers = 0, activeLoans = 0, todayLoans = 0, late = 0, waiting = 0;
};

// Résultat d'une opération d'écriture
struct Outcome {
    bool ok = false;
    QString error;
    QStringList promoted;   // lecteurs devenus prioritaires suite à l'opération
    int id = 0;
};

namespace Data {
// --- Connexion
bool open(const QString &path, QString *error = nullptr);
bool isEmpty();
void seedDemo();               // données de démonstration (base vide uniquement)

// --- Réglages de prêt (QSettings)
int loanDays();
int maxLoans();
void setLoanSettings(int days, int maxLoans);

// --- Lecture
QList<Book> books();
Book book(int id);
QList<Copy> copies(int bookId);
QList<Reader> readers();
QList<Loan> loans();
QList<WaitEntry> waitingFor(int bookId);
Stats stats();
QList<int> loansPerDay(int days);
QList<int> returnsPerDay(int days);
QString statusOf(const Book &b);   // Disponible / Réservé / Liste d'attente / Emprunté / Aucun exemplaire

// --- Écriture
Outcome addBook(const QString &title, const QString &author, const QString &isbn,
                const QStringList &barcodes);
Outcome updateBook(int id, const QString &title, const QString &author, const QString &isbn);
Outcome deleteBook(int id);
Outcome addCopy(int bookId, const QString &barcode);
Outcome deleteCopy(int copyId);
Outcome lend(int copyId, int readerId);
Outcome giveBack(int copyId);
Outcome joinWaitlist(int bookId, int readerId);
Outcome removeWait(int waitId);
Outcome addReader(const QString &name, const QString &matricule, const QString &phone);

// --- Formatage
QString fmt(const QDate &d);          // jj/MM/aaaa
QString dayName(const QDate &d);      // « Samedi »
QString monthName(int month);
QString group(int n);                 // 1248 -> « 1 248 »
}
