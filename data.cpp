#include "data.h"
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {

QString iso(const QDate &d) { return d.toString(Qt::ISODate); }
QString today() { return iso(QDate::currentDate()); }
QDate fromIso(const QVariant &v) { return v.isNull() ? QDate() : QDate::fromString(v.toString(), Qt::ISODate); }

// Exécute une requête et renvoie la première valeur entière.
int scalar(const QString &sql, const QVariantList &binds = {})
{
    QSqlQuery q;
    q.prepare(sql);
    for (const QVariant &b : binds) q.addBindValue(b);
    return (q.exec() && q.next()) ? q.value(0).toInt() : 0;
}

Outcome fail(const QString &msg)
{
    Outcome o;
    o.error = msg;
    return o;
}

Outcome success(int id = 0, const QStringList &promoted = {})
{
    Outcome o;
    o.ok = true;
    o.id = id;
    o.promoted = promoted;
    return o;
}

// Règle métier : au retour d'un exemplaire (ou ajout d'un exemplaire), les premiers
// lecteurs « En attente » deviennent « Prioritaires », à raison d'un par exemplaire disponible.
QStringList rebalance(int bookId)
{
    QStringList promoted;
    const int avail = scalar("SELECT COUNT(*) FROM exemplaire WHERE id_livre=? AND etat='Disponible'", {bookId});
    int prio = scalar("SELECT COUNT(*) FROM attente WHERE id_livre=? AND statut='Prioritaire'", {bookId});

    // Trop de prioritaires (exemplaire supprimé) : on repasse les derniers en attente.
    while (prio > avail) {
        QSqlQuery s;
        s.prepare("SELECT id_attente FROM attente WHERE id_livre=? AND statut='Prioritaire' "
                  "ORDER BY date_demande DESC, id_attente DESC LIMIT 1");
        s.addBindValue(bookId);
        if (!s.exec() || !s.next()) break;
        QSqlQuery u;
        u.prepare("UPDATE attente SET statut='En attente' WHERE id_attente=?");
        u.addBindValue(s.value(0));
        u.exec();
        --prio;
    }
    while (prio < avail) {
        QSqlQuery s;
        s.prepare("SELECT a.id_attente, l.nom FROM attente a JOIN lecteur l ON l.id_lecteur=a.id_lecteur "
                  "WHERE a.id_livre=? AND a.statut='En attente' "
                  "ORDER BY a.date_demande, a.id_attente LIMIT 1");
        s.addBindValue(bookId);
        if (!s.exec() || !s.next()) break;
        QSqlQuery u;
        u.prepare("UPDATE attente SET statut='Prioritaire' WHERE id_attente=?");
        u.addBindValue(s.value(0));
        u.exec();
        promoted << s.value(1).toString();
        ++prio;
    }
    return promoted;
}

} // namespace

namespace Data {

// ================================================================ Connexion
bool open(const QString &path, QString *error)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
        if (error) *error = db.lastError().text();
        return false;
    }
    QSqlQuery q;
    q.exec("PRAGMA foreign_keys = ON");
    const QStringList schema = {
        "CREATE TABLE IF NOT EXISTS livre ("
        " id_livre INTEGER PRIMARY KEY AUTOINCREMENT,"
        " titre TEXT NOT NULL, auteur TEXT NOT NULL, isbn TEXT, date_ajout TEXT)",
        "CREATE TABLE IF NOT EXISTS exemplaire ("
        " id_exemplaire INTEGER PRIMARY KEY AUTOINCREMENT,"
        " id_livre INTEGER NOT NULL REFERENCES livre(id_livre) ON DELETE CASCADE,"
        " code_barre TEXT NOT NULL UNIQUE,"
        " etat TEXT NOT NULL DEFAULT 'Disponible')",
        "CREATE TABLE IF NOT EXISTS lecteur ("
        " id_lecteur INTEGER PRIMARY KEY AUTOINCREMENT,"
        " nom TEXT NOT NULL, matricule TEXT NOT NULL UNIQUE, telephone TEXT)",
        "CREATE TABLE IF NOT EXISTS pret ("
        " id_pret INTEGER PRIMARY KEY AUTOINCREMENT,"
        " id_exemplaire INTEGER NOT NULL REFERENCES exemplaire(id_exemplaire) ON DELETE CASCADE,"
        " id_lecteur INTEGER NOT NULL REFERENCES lecteur(id_lecteur),"
        " date_pret TEXT NOT NULL, date_limite TEXT NOT NULL, date_retour TEXT)",
        "CREATE TABLE IF NOT EXISTS attente ("
        " id_attente INTEGER PRIMARY KEY AUTOINCREMENT,"
        " id_livre INTEGER NOT NULL REFERENCES livre(id_livre) ON DELETE CASCADE,"
        " id_lecteur INTEGER NOT NULL REFERENCES lecteur(id_lecteur),"
        " date_demande TEXT NOT NULL, statut TEXT NOT NULL DEFAULT 'En attente')",
    };
    for (const QString &sql : schema) {
        if (!q.exec(sql)) {
            if (error) *error = q.lastError().text();
            return false;
        }
    }
    return true;
}

bool isEmpty() { return scalar("SELECT COUNT(*) FROM livre") == 0 && scalar("SELECT COUNT(*) FROM lecteur") == 0; }

// ================================================================ Réglages
int loanDays() { return QSettings().value("pret/jours", 14).toInt(); }
int maxLoans() { return QSettings().value("pret/max", 5).toInt(); }
void setLoanSettings(int days, int max)
{
    QSettings s;
    s.setValue("pret/jours", days);
    s.setValue("pret/max", max);
}

// ================================================================ Lecture
QList<Book> books()
{
    QList<Book> list;
    QSqlQuery q;
    q.prepare(
        "SELECT l.id_livre, l.titre, l.auteur, COALESCE(l.isbn,''), l.date_ajout,"
        " (SELECT COUNT(*) FROM exemplaire e WHERE e.id_livre=l.id_livre),"
        " (SELECT COUNT(*) FROM exemplaire e WHERE e.id_livre=l.id_livre AND e.etat='Disponible'),"
        " (SELECT COUNT(*) FROM attente a WHERE a.id_livre=l.id_livre AND a.statut IN ('En attente','Prioritaire')),"
        " (SELECT COUNT(*) FROM attente a WHERE a.id_livre=l.id_livre AND a.statut='Prioritaire'),"
        " (SELECT COUNT(*) FROM pret p JOIN exemplaire e ON e.id_exemplaire=p.id_exemplaire"
        "   WHERE e.id_livre=l.id_livre AND p.date_pret >= ?),"
        " (SELECT MIN(p.date_limite) FROM pret p JOIN exemplaire e ON e.id_exemplaire=p.id_exemplaire"
        "   WHERE e.id_livre=l.id_livre AND p.date_retour IS NULL)"
        " FROM livre l ORDER BY l.id_livre");
    q.addBindValue(iso(QDate::currentDate().addDays(-30)));
    if (!q.exec()) return list;
    while (q.next()) {
        Book b;
        b.id = q.value(0).toInt();
        b.title = q.value(1).toString();
        b.author = q.value(2).toString();
        b.isbn = q.value(3).toString();
        b.added = fromIso(q.value(4));
        b.copies = q.value(5).toInt();
        b.avail = q.value(6).toInt();
        b.waiting = q.value(7).toInt();
        b.prio = q.value(8).toInt();
        b.loans30 = q.value(9).toInt();
        b.due = fromIso(q.value(10));
        list << b;
    }
    return list;
}

Book book(int id)
{
    for (const Book &b : books())
        if (b.id == id) return b;
    return Book();
}

QList<Copy> copies(int bookId)
{
    QList<Copy> list;
    QSqlQuery q;
    q.prepare("SELECT e.id_exemplaire, e.code_barre, e.etat, lec.nom, p.date_limite "
              "FROM exemplaire e "
              "LEFT JOIN pret p ON p.id_exemplaire=e.id_exemplaire AND p.date_retour IS NULL "
              "LEFT JOIN lecteur lec ON lec.id_lecteur=p.id_lecteur "
              "WHERE e.id_livre=? ORDER BY e.id_exemplaire");
    q.addBindValue(bookId);
    if (!q.exec()) return list;
    while (q.next()) {
        Copy c;
        c.id = q.value(0).toInt();
        c.bookId = bookId;
        c.barcode = q.value(1).toString();
        c.state = q.value(2).toString();
        c.borrower = q.value(3).toString();
        c.due = fromIso(q.value(4));
        list << c;
    }
    return list;
}

QList<Reader> readers()
{
    QList<Reader> list;
    QSqlQuery q;
    q.prepare("SELECT l.id_lecteur, l.nom, l.matricule, COALESCE(l.telephone,''),"
              " (SELECT COUNT(*) FROM pret p WHERE p.id_lecteur=l.id_lecteur AND p.date_retour IS NULL),"
              " (SELECT COUNT(*) FROM pret p WHERE p.id_lecteur=l.id_lecteur AND p.date_retour IS NULL AND p.date_limite < ?)"
              " FROM lecteur l ORDER BY l.nom");
    q.addBindValue(today());
    if (!q.exec()) return list;
    while (q.next()) {
        Reader r;
        r.id = q.value(0).toInt();
        r.name = q.value(1).toString();
        r.matricule = q.value(2).toString();
        r.phone = q.value(3).toString();
        r.loans = q.value(4).toInt();
        r.late = q.value(5).toInt();
        list << r;
    }
    return list;
}

QList<Loan> loans()
{
    QList<Loan> list;
    QSqlQuery q;
    q.exec("SELECT p.id_pret, p.id_exemplaire, lec.nom, l.titre, e.code_barre,"
           " p.date_pret, p.date_limite, p.date_retour "
           "FROM pret p JOIN exemplaire e ON e.id_exemplaire=p.id_exemplaire "
           "JOIN livre l ON l.id_livre=e.id_livre JOIN lecteur lec ON lec.id_lecteur=p.id_lecteur "
           "ORDER BY (p.date_retour IS NOT NULL),"
           " CASE WHEN p.date_retour IS NULL THEN p.date_limite END ASC,"
           " p.date_retour DESC, p.id_pret DESC LIMIT 300");
    const QDate now = QDate::currentDate();
    while (q.next()) {
        Loan l;
        l.id = q.value(0).toInt();
        l.copyId = q.value(1).toInt();
        l.reader = q.value(2).toString();
        l.book = q.value(3).toString();
        l.barcode = q.value(4).toString();
        l.loaned = fromIso(q.value(5));
        l.due = fromIso(q.value(6));
        l.returned = fromIso(q.value(7));
        l.status = l.returned.isValid() ? "Rendu" : (l.due < now ? "En retard" : "En cours");
        list << l;
    }
    return list;
}

QList<WaitEntry> waitingFor(int bookId)
{
    QList<WaitEntry> list;
    QSqlQuery q;
    q.prepare("SELECT a.id_attente, a.id_lecteur, lec.nom, l.titre, a.date_demande, a.statut "
              "FROM attente a JOIN lecteur lec ON lec.id_lecteur=a.id_lecteur "
              "JOIN livre l ON l.id_livre=a.id_livre "
              "WHERE a.id_livre=? AND a.statut IN ('En attente','Prioritaire') "
              "ORDER BY a.date_demande, a.id_attente");
    q.addBindValue(bookId);
    if (!q.exec()) return list;
    int pos = 1;
    while (q.next()) {
        WaitEntry w;
        w.id = q.value(0).toInt();
        w.bookId = bookId;
        w.readerId = q.value(1).toInt();
        w.reader = q.value(2).toString();
        w.book = q.value(3).toString();
        w.date = fromIso(q.value(4));
        w.status = q.value(5).toString();
        w.position = pos++;
        list << w;
    }
    return list;
}

Stats stats()
{
    Stats s;
    s.books = scalar("SELECT COUNT(*) FROM livre");
    s.copies = scalar("SELECT COUNT(*) FROM exemplaire");
    s.readers = scalar("SELECT COUNT(*) FROM lecteur");
    s.activeLoans = scalar("SELECT COUNT(*) FROM pret WHERE date_retour IS NULL");
    s.todayLoans = scalar("SELECT COUNT(*) FROM pret WHERE date_pret=?", {today()});
    s.late = scalar("SELECT COUNT(*) FROM pret WHERE date_retour IS NULL AND date_limite < ?", {today()});
    s.waiting = scalar("SELECT COUNT(*) FROM attente WHERE statut IN ('En attente','Prioritaire')");
    return s;
}

QList<int> loansPerDay(int days)
{
    QList<int> out;
    for (int i = days - 1; i >= 0; --i)
        out << scalar("SELECT COUNT(*) FROM pret WHERE date_pret=?", {iso(QDate::currentDate().addDays(-i))});
    return out;
}

QList<int> returnsPerDay(int days)
{
    QList<int> out;
    for (int i = days - 1; i >= 0; --i)
        out << scalar("SELECT COUNT(*) FROM pret WHERE date_retour=?", {iso(QDate::currentDate().addDays(-i))});
    return out;
}

QString statusOf(const Book &b)
{
    if (b.copies == 0) return QStringLiteral("Aucun exemplaire");
    if (b.avail > b.prio) return QStringLiteral("Disponible");
    if (b.avail > 0) return QStringLiteral("Réservé");   // les exemplaires libres attendent leurs lecteurs prioritaires
    return b.waiting > 0 ? QStringLiteral("Liste d'attente") : QStringLiteral("Emprunté");
}

// ================================================================ Écriture
Outcome addBook(const QString &title, const QString &author, const QString &isbn, const QStringList &barcodes)
{
    if (title.trimmed().isEmpty() || author.trimmed().isEmpty())
        return fail("Le titre et l'auteur sont obligatoires.");

    QStringList codes;
    for (const QString &c : barcodes) {
        const QString t = c.trimmed();
        if (t.isEmpty()) return fail("Saisissez le code-barres de chaque exemplaire.");
        if (codes.contains(t)) return fail("Le code-barres « " + t + " » est saisi deux fois.");
        if (scalar("SELECT COUNT(*) FROM exemplaire WHERE code_barre=?", {t}) > 0)
            return fail("Le code-barres « " + t + " » existe déjà.");
        codes << t;
    }

    QSqlDatabase::database().transaction();
    QSqlQuery q;
    q.prepare("INSERT INTO livre (titre, auteur, isbn, date_ajout) VALUES (?,?,?,?)");
    q.addBindValue(title.trimmed());
    q.addBindValue(author.trimmed());
    q.addBindValue(isbn.trimmed());
    q.addBindValue(today());
    if (!q.exec()) {
        QSqlDatabase::database().rollback();
        return fail(q.lastError().text());
    }
    const int id = q.lastInsertId().toInt();
    for (const QString &c : codes) {
        QSqlQuery e;
        e.prepare("INSERT INTO exemplaire (id_livre, code_barre, etat) VALUES (?,?, 'Disponible')");
        e.addBindValue(id);
        e.addBindValue(c);
        if (!e.exec()) {
            QSqlDatabase::database().rollback();
            return fail(e.lastError().text());
        }
    }
    QSqlDatabase::database().commit();
    return success(id);
}

Outcome updateBook(int id, const QString &title, const QString &author, const QString &isbn)
{
    if (title.trimmed().isEmpty() || author.trimmed().isEmpty())
        return fail("Le titre et l'auteur sont obligatoires.");
    QSqlQuery q;
    q.prepare("UPDATE livre SET titre=?, auteur=?, isbn=? WHERE id_livre=?");
    q.addBindValue(title.trimmed());
    q.addBindValue(author.trimmed());
    q.addBindValue(isbn.trimmed());
    q.addBindValue(id);
    return q.exec() ? success(id) : fail(q.lastError().text());
}

Outcome deleteBook(int id)
{
    if (scalar("SELECT COUNT(*) FROM pret p JOIN exemplaire e ON e.id_exemplaire=p.id_exemplaire "
               "WHERE e.id_livre=? AND p.date_retour IS NULL", {id}) > 0)
        return fail("Ce livre a des exemplaires empruntés. Rendez-les d'abord.");
    QSqlQuery q;
    q.prepare("DELETE FROM livre WHERE id_livre=?");   // supprime en cascade exemplaires, prêts et attentes
    q.addBindValue(id);
    return q.exec() ? success() : fail(q.lastError().text());
}

Outcome addCopy(int bookId, const QString &barcode)
{
    const QString code = barcode.trimmed();
    if (code.isEmpty()) return fail("Saisissez le code-barres de l'exemplaire.");
    if (scalar("SELECT COUNT(*) FROM exemplaire WHERE code_barre=?", {code}) > 0)
        return fail("Le code-barres « " + code + " » existe déjà.");
    QSqlQuery q;
    q.prepare("INSERT INTO exemplaire (id_livre, code_barre, etat) VALUES (?,?, 'Disponible')");
    q.addBindValue(bookId);
    q.addBindValue(code);
    if (!q.exec()) return fail(q.lastError().text());
    return success(q.lastInsertId().toInt(), rebalance(bookId));
}

Outcome deleteCopy(int copyId)
{
    QSqlQuery s;
    s.prepare("SELECT id_livre, etat FROM exemplaire WHERE id_exemplaire=?");
    s.addBindValue(copyId);
    if (!s.exec() || !s.next()) return fail("Exemplaire introuvable.");
    const int bookId = s.value(0).toInt();
    if (s.value(1).toString() != "Disponible")
        return fail("Cet exemplaire est emprunté. Rendez-le d'abord.");
    QSqlQuery q;
    q.prepare("DELETE FROM exemplaire WHERE id_exemplaire=?");
    q.addBindValue(copyId);
    if (!q.exec()) return fail(q.lastError().text());
    rebalance(bookId);
    return success();
}

Outcome lend(int copyId, int readerId)
{
    QSqlQuery s;
    s.prepare("SELECT id_livre, etat FROM exemplaire WHERE id_exemplaire=?");
    s.addBindValue(copyId);
    if (!s.exec() || !s.next()) return fail("Exemplaire introuvable.");
    const int bookId = s.value(0).toInt();
    if (s.value(1).toString() != "Disponible") return fail("Cet exemplaire n'est pas disponible.");
    if (scalar("SELECT COUNT(*) FROM lecteur WHERE id_lecteur=?", {readerId}) == 0)
        return fail("Lecteur introuvable.");

    if (scalar("SELECT COUNT(*) FROM pret WHERE id_lecteur=? AND date_retour IS NULL", {readerId}) >= maxLoans())
        return fail(QString("Ce lecteur a atteint la limite de %1 prêts en cours.").arg(maxLoans()));
    if (scalar("SELECT COUNT(*) FROM pret p JOIN exemplaire e ON e.id_exemplaire=p.id_exemplaire "
               "WHERE p.id_lecteur=? AND e.id_livre=? AND p.date_retour IS NULL", {readerId, bookId}) > 0)
        return fail("Ce lecteur a déjà un exemplaire de ce livre.");

    // Règle d'attente : les exemplaires libres reviennent d'abord aux lecteurs prioritaires.
    const bool isPrio = scalar("SELECT COUNT(*) FROM attente WHERE id_livre=? AND id_lecteur=? AND statut='Prioritaire'",
                               {bookId, readerId}) > 0;
    const int avail = scalar("SELECT COUNT(*) FROM exemplaire WHERE id_livre=? AND etat='Disponible'", {bookId});
    const int prio = scalar("SELECT COUNT(*) FROM attente WHERE id_livre=? AND statut='Prioritaire'", {bookId});
    if (!isPrio && avail - prio <= 0)
        return fail("Les exemplaires disponibles sont réservés aux lecteurs prioritaires de la liste d'attente.");

    QSqlDatabase::database().transaction();
    QSqlQuery p;
    p.prepare("INSERT INTO pret (id_exemplaire, id_lecteur, date_pret, date_limite) VALUES (?,?,?,?)");
    p.addBindValue(copyId);
    p.addBindValue(readerId);
    p.addBindValue(today());
    p.addBindValue(iso(QDate::currentDate().addDays(loanDays())));
    QSqlQuery u1, u2;
    u1.prepare("UPDATE exemplaire SET etat='Emprunté' WHERE id_exemplaire=?");
    u1.addBindValue(copyId);
    u2.prepare("UPDATE attente SET statut='Satisfaite' WHERE id_livre=? AND id_lecteur=? "
               "AND statut IN ('En attente','Prioritaire')");
    u2.addBindValue(bookId);
    u2.addBindValue(readerId);
    if (!p.exec() || !u1.exec() || !u2.exec()) {
        const QString err = p.lastError().text() + u1.lastError().text() + u2.lastError().text();
        QSqlDatabase::database().rollback();
        return fail(err);
    }
    QSqlDatabase::database().commit();
    rebalance(bookId);
    return success(p.lastInsertId().toInt());
}

Outcome giveBack(int copyId)
{
    QSqlQuery s;
    s.prepare("SELECT p.id_pret, e.id_livre FROM pret p JOIN exemplaire e ON e.id_exemplaire=p.id_exemplaire "
              "WHERE p.id_exemplaire=? AND p.date_retour IS NULL");
    s.addBindValue(copyId);
    if (!s.exec() || !s.next()) return fail("Aucun prêt en cours pour cet exemplaire.");
    const int loanId = s.value(0).toInt();
    const int bookId = s.value(1).toInt();

    QSqlDatabase::database().transaction();
    QSqlQuery u1, u2;
    u1.prepare("UPDATE pret SET date_retour=? WHERE id_pret=?");
    u1.addBindValue(today());
    u1.addBindValue(loanId);
    u2.prepare("UPDATE exemplaire SET etat='Disponible' WHERE id_exemplaire=?");
    u2.addBindValue(copyId);
    if (!u1.exec() || !u2.exec()) {
        QSqlDatabase::database().rollback();
        return fail(u1.lastError().text() + u2.lastError().text());
    }
    QSqlDatabase::database().commit();
    return success(loanId, rebalance(bookId));
}

Outcome joinWaitlist(int bookId, int readerId)
{
    const Book b = book(bookId);
    if (b.id == 0 || b.copies == 0) return fail("Ce livre n'a aucun exemplaire.");
    if (b.avail - b.prio > 0) return fail("Des exemplaires sont disponibles : empruntez-en un directement.");
    if (scalar("SELECT COUNT(*) FROM lecteur WHERE id_lecteur=?", {readerId}) == 0)
        return fail("Lecteur introuvable.");
    if (scalar("SELECT COUNT(*) FROM attente WHERE id_livre=? AND id_lecteur=? AND statut IN ('En attente','Prioritaire')",
               {bookId, readerId}) > 0)
        return fail("Ce lecteur est déjà dans la liste d'attente de ce livre.");
    if (scalar("SELECT COUNT(*) FROM pret p JOIN exemplaire e ON e.id_exemplaire=p.id_exemplaire "
               "WHERE p.id_lecteur=? AND e.id_livre=? AND p.date_retour IS NULL", {readerId, bookId}) > 0)
        return fail("Ce lecteur a déjà un exemplaire de ce livre.");

    QSqlQuery q;
    q.prepare("INSERT INTO attente (id_livre, id_lecteur, date_demande, statut) VALUES (?,?,?, 'En attente')");
    q.addBindValue(bookId);
    q.addBindValue(readerId);
    q.addBindValue(today());
    return q.exec() ? success(q.lastInsertId().toInt()) : fail(q.lastError().text());
}

Outcome removeWait(int waitId)
{
    QSqlQuery s;
    s.prepare("SELECT id_livre FROM attente WHERE id_attente=?");
    s.addBindValue(waitId);
    if (!s.exec() || !s.next()) return fail("Demande introuvable.");
    const int bookId = s.value(0).toInt();
    QSqlQuery q;
    q.prepare("DELETE FROM attente WHERE id_attente=?");
    q.addBindValue(waitId);
    if (!q.exec()) return fail(q.lastError().text());
    return success(0, rebalance(bookId));
}

Outcome addReader(const QString &name, const QString &matricule, const QString &phone)
{
    if (name.trimmed().isEmpty() || matricule.trimmed().isEmpty())
        return fail("Le nom et le matricule sont obligatoires.");
    if (scalar("SELECT COUNT(*) FROM lecteur WHERE matricule=?", {matricule.trimmed()}) > 0)
        return fail("Ce matricule existe déjà.");
    QSqlQuery q;
    q.prepare("INSERT INTO lecteur (nom, matricule, telephone) VALUES (?,?,?)");
    q.addBindValue(name.trimmed());
    q.addBindValue(matricule.trimmed());
    q.addBindValue(phone.trimmed());
    return q.exec() ? success(q.lastInsertId().toInt()) : fail(q.lastError().text());
}

// ================================================================ Formatage
QString fmt(const QDate &d) { return d.isValid() ? d.toString("dd/MM/yyyy") : QStringLiteral("—"); }

QString dayName(const QDate &d)
{
    static const QStringList n = {"Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"};
    return n.at(d.dayOfWeek() - 1);
}

QString monthName(int m)
{
    static const QStringList n = {"janvier", "février", "mars", "avril", "mai", "juin", "juillet",
                                  "août", "septembre", "octobre", "novembre", "décembre"};
    return n.at(m - 1);
}

QString group(int n)
{
    QString s = QString::number(n);
    for (int i = s.size() - 3; i > 0; i -= 3) s.insert(i, ' ');
    return s;
}

// ================================================================ Démo
void seedDemo()
{
    if (!isEmpty()) return;
    const QDate now = QDate::currentDate();
    QSqlDatabase::database().transaction();

    const QStringList names = {
        "Rakoto Andry", "Rasoamanana Hanta", "Randria Fenitra", "Rabe Tiana", "Andrianina Mamy",
        "Razafy Lalaina", "Rasolofo Miora", "Ravelo Tahina", "Ranaivo Faly", "Rakotomalala Sitraka",
        "Randrianasolo Voahangy", "Rakotonirina Hery", "Andriamanana Sarobidy", "Rasoanaivo Noro",
        "Rajaonarison Toky", "Razanakoto Lova"};
    QList<int> readerIds;
    for (int i = 0; i < names.size(); ++i) {
        QSqlQuery q;
        q.prepare("INSERT INTO lecteur (nom, matricule, telephone) VALUES (?,?,?)");
        q.addBindValue(names[i]);
        q.addBindValue(QString("%1-%2").arg(i % 5 == 3 ? "P" : "E").arg(142 + i * 17, 4, 10, QChar('0')));
        q.addBindValue(QString("034 %1 %2 %3").arg(10 + i).arg(300 + i * 7).arg(40 + i));
        q.exec();
        readerIds << q.lastInsertId().toInt();
    }

    struct B { const char *title, *author; int copies, avail, waiting, addedAgo, dueBase; };
    const QList<B> list = {
        {"Clean Code", "Robert C. Martin", 5, 2, 0, 1, 9},
        {"L'Art de la programmation", "Donald E. Knuth", 3, 0, 6, 2, 3},
        {"Intelligence artificielle", "Stéphane Caron", 4, 1, 0, 3, 7},
        {"Réseaux et sécurité", "Jean-Philippe Romain", 3, 3, 0, 4, 5},
        {"Introduction à l'algorithmique", "Thomas H. Cormen", 6, 0, 9, 12, 2},
        {"Systèmes d'exploitation", "Andrew Tanenbaum", 4, 0, 4, 20, -4},
        {"Apprentissage automatique", "Aurélien Géron", 3, 0, 5, 9, -2},
        {"Structures de données", "Robert Sedgewick", 3, 0, 3, 30, 6},
        {"Bases de données", "Georges Gardarin", 5, 4, 0, 45, 8},
        {"Génie logiciel", "Ian Sommerville", 4, 2, 0, 60, 11},
        {"Python pour la data science", "Jake VanderPlas", 4, 0, 0, 15, 4},
        {"Mathématiques discrètes", "Kenneth Rosen", 3, 1, 0, 80, 10},
    };

    QList<int> allCopyIds;
    int barcode = 30001, cursor = 0, bookNo = 0;
    for (const B &b : list) {
        QSqlQuery q;
        q.prepare("INSERT INTO livre (titre, auteur, isbn, date_ajout) VALUES (?,?,?,?)");
        q.addBindValue(QString::fromUtf8(b.title));
        q.addBindValue(QString::fromUtf8(b.author));
        q.addBindValue(QString("978-2-%1-%2-%3").arg(1000 + bookNo * 37).arg(200 + bookNo * 11).arg(bookNo % 10));
        q.addBindValue(iso(now.addDays(-b.addedAgo)));
        q.exec();
        const int bookId = q.lastInsertId().toInt();
        QList<int> holders;
        const int borrowed = b.copies - b.avail;
        for (int k = 0; k < b.copies; ++k) {
            const bool out = k < borrowed;
            QSqlQuery e;
            e.prepare("INSERT INTO exemplaire (id_livre, code_barre, etat) VALUES (?,?,?)");
            e.addBindValue(bookId);
            e.addBindValue(QString("BM%1").arg(barcode++, 6, 10, QChar('0')));
            e.addBindValue(out ? "Emprunté" : "Disponible");
            e.exec();
            const int copyId = e.lastInsertId().toInt();
            allCopyIds << copyId;
            if (out) {
                const int reader = readerIds[cursor++ % readerIds.size()];
                holders << reader;
                const QDate due = now.addDays(b.dueBase + k * 2);
                QSqlQuery p;
                p.prepare("INSERT INTO pret (id_exemplaire, id_lecteur, date_pret, date_limite) VALUES (?,?,?,?)");
                p.addBindValue(copyId);
                p.addBindValue(reader);
                p.addBindValue(iso(due.addDays(-loanDays())));
                p.addBindValue(iso(due));
                p.exec();
            }
        }
        int added = 0;
        for (int i = 0; i < readerIds.size() && added < b.waiting; ++i) {
            const int reader = readerIds[(bookNo * 5 + i) % readerIds.size()];
            if (holders.contains(reader)) continue;
            QSqlQuery w;
            w.prepare("INSERT INTO attente (id_livre, id_lecteur, date_demande, statut) VALUES (?,?,?, 'En attente')");
            w.addBindValue(bookId);
            w.addBindValue(reader);
            w.addBindValue(iso(now.addDays(-(b.waiting - added))));
            if (w.exec()) ++added;
        }
        ++bookNo;
    }

    // Historique de prêts rendus (alimente le graphique d'activité)
    const QList<int> perDay = {20, 17, 24, 19, 23, 18, 21, 14, 22, 17, 25, 40, 26, 32};
    int n = 0;
    for (int d = 0; d < perDay.size(); ++d) {
        const QDate day = now.addDays(-(perDay.size() - 1 - d));
        for (int i = 0; i < perDay[d]; ++i, ++n) {
            QSqlQuery p;
            p.prepare("INSERT INTO pret (id_exemplaire, id_lecteur, date_pret, date_limite, date_retour) VALUES (?,?,?,?,?)");
            p.addBindValue(allCopyIds[n % allCopyIds.size()]);
            p.addBindValue(readerIds[(n * 3) % readerIds.size()]);
            p.addBindValue(iso(day));
            p.addBindValue(iso(day.addDays(loanDays())));
            const QDate back = day.addDays(n % 3);       // rendu 0 à 2 jours plus tard
            p.addBindValue(iso(back > now ? day : back));   // jamais dans le futur
            p.exec();
        }
    }
    QSqlDatabase::database().commit();
}

} // namespace Data
