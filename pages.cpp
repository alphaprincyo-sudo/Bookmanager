#include "pages.h"
#include "data.h"
#include "dialogs.h"
#include "theme.h"
#include "widgets.h"
#include "ui_parametrespage.h"
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTableWidget>
#include <algorithm>

namespace {
QPushButton *actionButton(const QString &text, const char *kind)
{
    auto *b = new QPushButton(text);
    b->setObjectName(kind);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}
}

// ================================================================ HomePage
HomePage::HomePage(QWidget *parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    refresh();
}

void HomePage::refresh()
{
    auto *inner = new QFrame;
    auto *root = new QVBoxLayout(inner);
    root->setContentsMargins(24, 20, 24, 24);
    root->setSpacing(18);

    const QDate today = QDate::currentDate();
    const Stats st = Data::stats();
    const QList<Book> books = Data::books();

    // --- Bandeau (image fournie) + date
    auto *top = new QHBoxLayout;
    top->setSpacing(16);
    auto *banner = new ImageBanner(":/icons/banner_card.png");
    banner->setFixedHeight(130);
    auto *bl = new QHBoxLayout(banner);
    bl->setContentsMargins(28, 0, 10, 0);
    auto *txt = new QVBoxLayout;
    txt->setSpacing(4);
    txt->addStretch();
    auto *hello = new QLabel("Bonjour, Bibliothécaire !");
    hello->setObjectName("hello");
    auto *helloSub = new QLabel("Voici un aperçu de l'activité de votre bibliothèque.");
    helloSub->setObjectName("helloSub");
    txt->addWidget(hello);
    txt->addWidget(helloSub);
    txt->addStretch();
    bl->addLayout(txt);
    bl->addStretch();
    top->addWidget(banner, 1);

    auto *chip = new QFrame;
    chip->setObjectName("dateChip");
    chip->setFixedHeight(130);
    auto *cl = new QHBoxLayout(chip);
    cl->setContentsMargins(16, 10, 18, 10);
    cl->setSpacing(12);
    cl->addWidget(Theme::iconLabel("calendar", QColor("#2f6a4a"), 28));
    auto *dl = new QVBoxLayout;
    dl->setSpacing(0);
    dl->addStretch();
    auto *d1 = new QLabel(Data::dayName(today));
    auto *d2 = new QLabel(QString("%1 %2 %3").arg(today.day()).arg(Data::monthName(today.month())).arg(today.year()));
    d1->setObjectName("dateDay");
    d2->setObjectName("dateDay");
    dl->addWidget(d1);
    dl->addWidget(d2);
    dl->addStretch();
    cl->addLayout(dl);
    top->addWidget(chip);
    root->addLayout(top);

    // --- Statistiques (cliquables)
    auto *stats = new QHBoxLayout;
    stats->setSpacing(14);
    struct S { const char *ic, *col; QString label, value, sub; int target; };
    const QList<S> cards = {
        {"book", "#2f6a4a", "Total des livres", Data::group(st.books), QString("dont %1 exemplaires").arg(Data::group(st.copies)), PageLivres},
        {"user", "#4a7fc0", "Lecteurs inscrits", Data::group(st.readers), "Étudiants et personnel", PageLecteurs},
        {"swap", "#2f6a4a", "Emprunts en cours", Data::group(st.activeLoans), QString("+ %1 aujourd'hui").arg(st.todayLoans), PageEmprunts},
        {"alert", "#e0735a", "Livres en retard", Data::group(st.late), "À relancer", PageEmprunts},
        {"hourglass", "#8a78c8", "Demandes en attente", Data::group(st.waiting), "Dans la liste d'attente", PageAttente},
    };
    for (const S &s : cards) {
        auto *c = new StatCard(s.ic, s.col, s.label, s.value, s.sub);
        const int target = s.target;
        connect(c, &StatCard::clicked, this, [this, target] { emit navigate(target); });
        stats->addWidget(c, 1);
    }
    root->addLayout(stats);

    // --- Activité récente + derniers livres
    auto *mid = new QHBoxLayout;
    mid->setSpacing(16);
    auto *combo = new QComboBox;
    combo->addItems({"7 derniers jours", "14 derniers jours"});
    combo->setCurrentIndex(m_days == 7 ? 0 : 1);
    auto *chart = new ActivityChart;
    chart->setSeries(Data::loansPerDay(m_days), Data::returnsPerDay(m_days));
    connect(combo, &QComboBox::currentIndexChanged, this, [this, chart](int i) {
        m_days = i == 0 ? 7 : 14;
        chart->setSeries(Data::loansPerDay(m_days), Data::returnsPerDay(m_days));
    });
    auto *act = new SectionCard("Activité récente", "chart", QString(), combo);
    act->body()->addWidget(chart);
    mid->addWidget(act, 3);

    auto *latest = new SectionCard("Derniers livres ajoutés", "book", "Voir tout");
    connect(latest, &SectionCard::actionClicked, this, [this] { emit navigate(PageLivres); });
    QList<Book> byDate = books;
    std::sort(byDate.begin(), byDate.end(), [](const Book &a, const Book &b) { return a.added > b.added || (a.added == b.added && a.id > b.id); });
    for (int i = 0; i < qMin(4, int(byDate.size())); ++i) {
        auto *r = new BookRow(byDate[i], "Ajouté le " + Data::fmt(byDate[i].added));
        connect(r, &BookRow::clicked, this, [this] { emit navigate(PageLivres); });
        latest->body()->addWidget(r);
    }
    if (byDate.isEmpty()) latest->body()->addWidget(Theme::muted("Aucun livre pour le moment."));
    latest->body()->addStretch();
    mid->addWidget(latest, 2);
    root->addLayout(mid);

    // --- Populaires + plus attendus
    auto *row3 = new QHBoxLayout;
    row3->setSpacing(16);

    auto *pop = new SectionCard("Populaires ce mois-ci", "trend", "Voir tout");
    connect(pop, &SectionCard::actionClicked, this, [this] { emit navigate(PageLivres); });
    QList<Book> byLoans = books;
    std::sort(byLoans.begin(), byLoans.end(), [](const Book &a, const Book &b) { return a.loans30 > b.loans30; });
    int rank = 0;
    for (const Book &b : byLoans) {
        if (b.loans30 == 0 || rank == 4) break;
        auto *r = new BookRow(b, QString("%1 emprunts ce mois-ci").arg(b.loans30), Data::statusOf(b), ++rank);
        connect(r, &BookRow::clicked, this, [this] { emit navigate(PageLivres); });
        pop->body()->addWidget(r);
    }
    if (rank == 0) pop->body()->addWidget(Theme::muted("Aucun emprunt ce mois-ci."));
    row3->addWidget(pop, 1);

    auto *wait = new SectionCard("Les plus attendus", "queue", "Voir la liste d'attente");
    connect(wait, &SectionCard::actionClicked, this, [this] { emit navigate(PageAttente); });
    QList<Book> byWait = books;
    std::sort(byWait.begin(), byWait.end(), [](const Book &a, const Book &b) { return a.waiting > b.waiting; });
    rank = 0;
    for (const Book &b : byWait) {
        if (b.waiting == 0 || rank == 4) break;
        const QString ret = b.due.isValid() && b.avail == 0 ? " • retour prévu le " + b.due.toString("dd/MM") : QString();
        auto *r = new BookRow(b, QString("%1 en attente%2").arg(b.waiting).arg(ret), "Liste d'attente", ++rank);
        connect(r, &BookRow::clicked, this, [this] { emit navigate(PageAttente); });
        wait->body()->addWidget(r);
    }
    if (rank == 0) wait->body()->addWidget(Theme::muted("Personne n'attend de livre."));
    row3->addWidget(wait, 1);
    root->addLayout(row3);

    // --- Citation
    auto *q = new QFrame;
    q->setObjectName("card");
    auto *ql = new QHBoxLayout(q);
    ql->setContentsMargins(18, 12, 18, 12);
    ql->addWidget(Theme::iconLabel("leaf", QColor("#7a9a7e"), 28));
    ql->addSpacing(8);
    auto *quote = new QLabel("« Une bibliothèque, c'est un lieu où la nature et l'esprit se rencontrent. »");
    quote->setObjectName("quote");
    ql->addWidget(quote);
    ql->addStretch();
    root->addWidget(q);
    root->addStretch();

    if (QWidget *old = takeWidget()) old->deleteLater();
    setWidget(inner);
}

// ================================================================ LivresPage
LivresPage::LivresPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *lay;
    auto *scroll = pageShell("Livres",
                             "Catalogue de la bibliothèque. Double-cliquez sur un livre pour voir ses exemplaires.", lay);
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    auto *bar = new QHBoxLayout;
    bar->setSpacing(8);
    for (const QString &name : {"Tous", "Disponible", "Emprunté", "Liste d'attente"}) {
        auto *b = new QPushButton(name);
        b->setObjectName("chip");
        b->setCheckable(true);
        b->setAutoExclusive(true);
        b->setChecked(name == "Tous");
        b->setCursor(Qt::PointingHandCursor);
        connect(b, &QPushButton::clicked, this, [this, name] { m_filter = name; refresh(); });
        bar->addWidget(b);
    }
    bar->addStretch();
    auto *add = actionButton("+ Ajouter un livre", "primary");
    connect(add, &QPushButton::clicked, this, &LivresPage::addBook);
    bar->addWidget(add);
    lay->addLayout(bar);

    m_grid = new QGridLayout;
    m_grid->setSpacing(14);
    m_grid->setColumnStretch(0, 1);
    m_grid->setColumnStretch(1, 1);
    lay->addLayout(m_grid);
    lay->addStretch();
    refresh();
}

void LivresPage::setQuery(const QString &text)
{
    m_query = text.trimmed().toLower();
    refresh();
}

void LivresPage::refresh()
{
    clearLayout(m_grid);
    int i = 0;
    for (const Book &b : Data::books()) {
        if (m_filter != "Tous" && Data::statusOf(b) != m_filter) continue;
        if (!m_query.isEmpty() && !b.title.toLower().contains(m_query)
            && !b.author.toLower().contains(m_query) && !b.isbn.toLower().contains(m_query))
            continue;
        auto *card = new BookCard(b);
        connect(card, &BookCard::doubleClicked, this, &LivresPage::openBook);
        m_grid->addWidget(card, i / 2, i % 2);
        ++i;
    }
    if (i == 0)
        m_grid->addWidget(Theme::muted(m_query.isEmpty() && m_filter == "Tous"
                                           ? "Aucun livre. Cliquez sur « + Ajouter un livre »."
                                           : "Aucun livre ne correspond à votre recherche."), 0, 0);
}

void LivresPage::openBook(int bookId)
{
    BookDialog dlg(bookId, this);
    dlg.exec();
    if (dlg.changed()) {
        refresh();
        emit dataChanged();
    }
}

void LivresPage::addBook()
{
    AddBookDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        refresh();
        emit dataChanged();
    }
}

// ================================================================ LecteursPage
LecteursPage::LecteursPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *lay;
    auto *scroll = pageShell("Lecteurs", "Étudiants et personnel inscrits à la bibliothèque.", lay);
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    auto *bar = new QHBoxLayout;
    bar->addStretch();
    auto *add = actionButton("+ Ajouter un lecteur", "primary");
    connect(add, &QPushButton::clicked, this, &LecteursPage::addReader);
    bar->addWidget(add);
    lay->addLayout(bar);

    auto *card = new QFrame;
    card->setObjectName("card");
    m_cardLayout = new QVBoxLayout(card);
    m_cardLayout->setContentsMargins(6, 6, 6, 6);
    lay->addWidget(card);
    lay->addStretch();
    refresh();
}

void LecteursPage::refresh()
{
    clearLayout(m_cardLayout);
    QList<QStringList> rows;
    for (const Reader &r : Data::readers())
        rows.append({r.matricule, r.name, r.phone.isEmpty() ? "—" : r.phone, QString::number(r.loans),
                     r.late > 0 ? "En retard" : "Actif"});
    if (rows.isEmpty())
        m_cardLayout->addWidget(Theme::muted("Aucun lecteur inscrit. Cliquez sur « + Ajouter un lecteur »."));
    else
        m_cardLayout->addWidget(makeTable({"Matricule", "Nom", "Téléphone", "Emprunts en cours", "Statut"}, rows, 4));
}

void LecteursPage::addReader()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Nouveau lecteur");
    dlg.setMinimumWidth(440);
    auto *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(22, 20, 22, 18);
    root->setSpacing(14);
    auto *head = new QLabel("Nouveau lecteur");
    head->setObjectName("cardTitle");
    root->addWidget(head);
    auto *form = new QFormLayout;
    form->setVerticalSpacing(10);
    auto *name = new QLineEdit, *matricule = new QLineEdit, *phone = new QLineEdit;
    form->addRow("Nom :", name);
    form->addRow("Matricule :", matricule);
    form->addRow("Téléphone :", phone);
    root->addLayout(form);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText("Ajouter");
    bb->button(QDialogButtonBox::Ok)->setObjectName("primary");
    bb->button(QDialogButtonBox::Cancel)->setText("Annuler");
    bb->button(QDialogButtonBox::Cancel)->setObjectName("smallGhost");
    root->addWidget(bb);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(bb, &QDialogButtonBox::accepted, &dlg, [&] {
        const Outcome o = Data::addReader(name->text(), matricule->text(), phone->text());
        if (!o.ok) { QMessageBox::warning(&dlg, "Impossible d'ajouter", o.error); return; }
        dlg.accept();
    });
    if (dlg.exec() == QDialog::Accepted) {
        refresh();
        emit dataChanged();
    }
}

// ================================================================ EmpruntsPage
EmpruntsPage::EmpruntsPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *lay;
    auto *scroll = pageShell("Emprunts", "Prêts en cours, retards et retours (les prêts à rendre apparaissent en premier).", lay);
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
    auto *card = new QFrame;
    card->setObjectName("card");
    m_cardLayout = new QVBoxLayout(card);
    m_cardLayout->setContentsMargins(6, 6, 6, 6);
    lay->addWidget(card);
    lay->addStretch();
    refresh();
}

void EmpruntsPage::refresh()
{
    clearLayout(m_cardLayout);
    const QList<Loan> loans = Data::loans();
    if (loans.isEmpty()) {
        m_cardLayout->addWidget(Theme::muted("Aucun prêt enregistré. Empruntez un exemplaire depuis la fiche d'un livre."));
        return;
    }
    auto *t = new QTableWidget(loans.size(), 7);
    t->setHorizontalHeaderLabels({"Lecteur", "Livre", "Code-barres", "Emprunté le", "Retour prévu", "Statut", "Action"});
    t->verticalHeader()->setVisible(false);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setShowGrid(false);
    t->setAlternatingRowColors(true);
    t->setFocusPolicy(Qt::NoFocus);
    auto *hh = t->horizontalHeader();
    hh->setHighlightSections(false);
    hh->setSectionResizeMode(0, QHeaderView::Stretch);
    hh->setSectionResizeMode(1, QHeaderView::Stretch);
    for (int c = 2; c < 7; ++c) hh->setSectionResizeMode(c, QHeaderView::Fixed);
    t->setColumnWidth(2, 110);
    t->setColumnWidth(3, 110);
    t->setColumnWidth(4, 110);
    t->setColumnWidth(5, 120);
    t->setColumnWidth(6, 110);
    for (int r = 0; r < loans.size(); ++r) {
        const Loan &l = loans[r];
        t->setRowHeight(r, 48);
        t->setItem(r, 0, new QTableWidgetItem(l.reader));
        t->setItem(r, 1, new QTableWidgetItem(l.book));
        t->setItem(r, 2, new QTableWidgetItem(l.barcode));
        t->setItem(r, 3, new QTableWidgetItem(Data::fmt(l.loaned)));
        t->setItem(r, 4, new QTableWidgetItem(Data::fmt(l.due)));
        auto *state = new QWidget;
        auto *sl = new QHBoxLayout(state);
        sl->setContentsMargins(8, 0, 8, 0);
        sl->addWidget(Theme::badge(l.status), 0, Qt::AlignLeft | Qt::AlignVCenter);
        t->setCellWidget(r, 5, state);
        if (l.status == "Rendu") {
            t->setItem(r, 6, new QTableWidgetItem("Rendu le " + l.returned.toString("dd/MM")));
        } else {
            auto *act = new QWidget;
            auto *al = new QHBoxLayout(act);
            al->setContentsMargins(8, 0, 8, 0);
            auto *ret = actionButton("Rendre", "smallPrimary");
            const int copyId = l.copyId;
            connect(ret, &QPushButton::clicked, this, [this, copyId] {
                const Outcome o = Data::giveBack(copyId);
                if (!o.ok) { QMessageBox::warning(this, "Action impossible", o.error); return; }
                if (!o.promoted.isEmpty())
                    QMessageBox::information(this, "Retour enregistré",
                                             "Exemplaire rendu.\n\n" + o.promoted.join(", ")
                                                 + " est désormais prioritaire : cet exemplaire lui est réservé.");
                refresh();
                emit dataChanged();
            });
            al->addWidget(ret);
            t->setCellWidget(r, 6, act);
        }
    }
    t->setMinimumHeight(48 * loans.size() + 44);
    m_cardLayout->addWidget(t);
}

// ================================================================ ListeAttentePage
ListeAttentePage::ListeAttentePage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *lay;
    auto *scroll = pageShell("Liste d'attente",
                             "Livres possédés par la bibliothèque mais actuellement indisponibles, "
                             "classés du plus attendu au moins attendu.", lay);
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
    m_content = new QVBoxLayout;
    m_content->setSpacing(16);
    lay->addLayout(m_content);
    lay->addStretch();
    refresh();
}

void ListeAttentePage::refresh()
{
    clearLayout(m_content);
    QList<Book> items;
    int total = 0;
    for (const Book &b : Data::books())
        if (b.copies > 0 && b.avail - b.prio <= 0) { items << b; total += b.waiting; }
    std::sort(items.begin(), items.end(), [](const Book &a, const Book &b) { return a.waiting > b.waiting; });

    m_content->addWidget(Theme::muted(QString("%1 livres indisponibles • %2 demandes en attente").arg(items.size()).arg(total)));
    if (items.isEmpty())
        m_content->addWidget(Theme::muted("Tous les livres ont au moins un exemplaire disponible."));

    for (const Book &b : items) {
        auto *card = new QFrame;
        card->setObjectName("card");
        auto *cl = new QHBoxLayout(card);
        cl->setContentsMargins(18, 16, 20, 16);
        cl->setSpacing(18);
        cl->addWidget(new Cover(b.title, 76, 106), 0, Qt::AlignTop);

        auto *info = new QVBoxLayout;
        info->setSpacing(4);
        auto *t = new QLabel(b.title);
        t->setObjectName("rowTitle");
        info->addWidget(t);
        info->addWidget(Theme::muted(b.author));
        info->addWidget(Theme::muted(QString("%1 / %2 exemplaires disponibles").arg(b.avail).arg(b.copies)));
        if (b.due.isValid())
            info->addWidget(Theme::muted("Prochain retour prévu le " + Data::fmt(b.due)));
        info->addSpacing(4);
        info->addWidget(Theme::badge(Data::statusOf(b)), 0, Qt::AlignLeft);
        info->addStretch();
        cl->addLayout(info, 1);

        auto *q = new QVBoxLayout;
        q->setSpacing(3);
        auto *head = new QLabel(b.waiting > 0 ? QString("%1 en file d'attente").arg(b.waiting) : QString("Personne en attente"));
        head->setStyleSheet("font-weight:600; font-size:15px; color:#5b4a9a;");
        q->addWidget(head);
        const QList<WaitEntry> queue = Data::waitingFor(b.id);
        for (int k = 0; k < qMin(3, int(queue.size())); ++k)
            q->addWidget(Theme::muted(QString("%1. %2%3").arg(queue[k].position).arg(queue[k].reader,
                                                                                    queue[k].status == "Prioritaire" ? "  (prioritaire)" : "")));
        if (queue.size() > 3)
            q->addWidget(Theme::muted(QString("+ %1 autres").arg(queue.size() - 3)));
        q->addSpacing(6);
        auto *open = actionButton("Voir la fiche", "smallGhost");
        const int id = b.id;
        connect(open, &QPushButton::clicked, this, [this, id] {
            BookDialog dlg(id, this);
            dlg.exec();
            if (dlg.changed()) {
                refresh();
                emit dataChanged();
            }
        });
        q->addWidget(open, 0, Qt::AlignLeft);
        q->addStretch();
        cl->addLayout(q);
        m_content->addWidget(card);
    }
}

// ================================================================ ParametresPage
ParametresPage::ParametresPage(QWidget *parent) : QWidget(parent), ui(new Ui::ParametresPage)
{
    ui->setupUi(this);
    ui->dureeSpin->setValue(Data::loanDays());
    ui->maxSpin->setValue(Data::maxLoans());
    ui->nameEdit->setText(QSettings().value("bibliotheque/nom", "Bibliothèque universitaire").toString());

    connect(ui->btnEnregistrer, &QPushButton::clicked, this, [this] {
        Data::setLoanSettings(ui->dureeSpin->value(), ui->maxSpin->value());
        QSettings().setValue("bibliotheque/nom", ui->nameEdit->text().trimmed());
        QMessageBox::information(this, "Paramètres", "Paramètres enregistrés.");
    });
    connect(ui->btnDemo, &QPushButton::clicked, this, [this] {
        if (!Data::isEmpty()) {
            QMessageBox::information(this, "Données de démonstration",
                                     "La base contient déjà des données : rien n'a été ajouté.");
            return;
        }
        Data::seedDemo();
        QMessageBox::information(this, "Données de démonstration", "Livres, lecteurs, prêts et attentes d'exemple ajoutés.");
        emit dataChanged();
    });
}

ParametresPage::~ParametresPage() { delete ui; }
