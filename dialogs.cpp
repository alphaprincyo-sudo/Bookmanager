#include "dialogs.h"
#include "theme.h"
#include "widgets.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
QPushButton *smallButton(const QString &text, const char *kind)
{
    auto *b = new QPushButton(text);
    b->setObjectName(kind);   // smallPrimary / smallGhost / smallDanger
    b->setCursor(Qt::PointingHandCursor);
    return b;
}
}

// ================================================================ AddBookDialog
AddBookDialog::AddBookDialog(QWidget *parent) : QDialog(parent)
{
    init("Ajouter un livre");
}

AddBookDialog::AddBookDialog(const Book &b, QWidget *parent) : QDialog(parent), m_edit(true), m_bookId(b.id)
{
    init("Modifier le livre");
    m_title->setText(b.title);
    m_author->setText(b.author);
    m_isbn->setText(b.isbn);
}

void AddBookDialog::init(const QString &title)
{
    setWindowTitle(title);
    setModal(true);
    setMinimumWidth(520);
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(22, 20, 22, 18);
    root->setSpacing(14);

    auto *head = new QLabel(title);
    head->setObjectName("cardTitle");
    root->addWidget(head);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(10);
    m_title = new QLineEdit;
    m_author = new QLineEdit;
    m_isbn = new QLineEdit;
    m_title->setPlaceholderText("Titre du livre");
    m_author->setPlaceholderText("Nom de l'auteur");
    m_isbn->setPlaceholderText("978-…  (facultatif)");
    form->addRow("Titre :", m_title);
    form->addRow("Auteur :", m_author);
    form->addRow("ISBN :", m_isbn);

    if (!m_edit) {
        m_count = new QSpinBox;
        m_count->setRange(0, 50);
        m_count->setValue(1);
        m_count->setSpecialValueText("0 (aucun pour l'instant)");
        form->addRow("Exemplaires :", m_count);
    }
    root->addLayout(form);

    if (!m_edit) {
        auto *hint = Theme::muted("Saisissez ou scannez le code-barres de chaque exemplaire. "
                                  "Ils sont « Disponible » par défaut.");
        hint->setWordWrap(true);
        root->addWidget(hint);

        m_codesBox = new QWidget;
        m_codesLayout = new QVBoxLayout(m_codesBox);
        m_codesLayout->setContentsMargins(0, 0, 0, 0);
        m_codesLayout->setSpacing(8);
        auto *scroll = new QScrollArea;
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setMaximumHeight(210);
        scroll->setWidget(m_codesBox);
        root->addWidget(scroll);
        connect(m_count, &QSpinBox::valueChanged, this, [this] { rebuildCodes(); });
        rebuildCodes();
    }

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText(m_edit ? "Enregistrer" : "Ajouter");
    bb->button(QDialogButtonBox::Ok)->setObjectName("primary");
    bb->button(QDialogButtonBox::Cancel)->setText("Annuler");
    bb->button(QDialogButtonBox::Cancel)->setObjectName("smallGhost");
    connect(bb, &QDialogButtonBox::accepted, this, &AddBookDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &AddBookDialog::reject);
    root->addWidget(bb);
    m_title->setFocus();
}

// Un champ code-barres par exemplaire ; on conserve ce qui est déjà saisi.
void AddBookDialog::rebuildCodes()
{
    const int n = m_count->value();
    while (m_codes.size() > n) {
        QLineEdit *e = m_codes.takeLast();
        m_codesLayout->removeWidget(e->parentWidget());
        e->parentWidget()->deleteLater();
    }
    while (m_codes.size() < n) {
        const int idx = m_codes.size() + 1;
        auto *row = new QWidget;
        auto *rl = new QHBoxLayout(row);
        rl->setContentsMargins(0, 0, 0, 0);
        auto *lbl = new QLabel(QString("Exemplaire %1").arg(idx));
        lbl->setMinimumWidth(100);
        auto *edit = new QLineEdit;
        edit->setPlaceholderText("Code-barres");
        rl->addWidget(lbl);
        rl->addWidget(edit, 1);
        rl->addWidget(Theme::badge("Disponible"));
        m_codesLayout->addWidget(row);
        // Entrée (comme en fin de scan) passe au champ suivant
        connect(edit, &QLineEdit::returnPressed, this, [this, edit] {
            const int i = m_codes.indexOf(edit);
            if (i >= 0 && i + 1 < m_codes.size()) m_codes[i + 1]->setFocus();
        });
        m_codes << edit;
    }
    m_codesBox->setVisible(n > 0);
    if (n > 0 && m_codes.first()->text().isEmpty() && n == 1) m_codes.first()->setFocus();
}

void AddBookDialog::accept()
{
    Outcome o;
    if (m_edit) {
        o = Data::updateBook(m_bookId, m_title->text(), m_author->text(), m_isbn->text());
    } else {
        QStringList codes;
        for (QLineEdit *e : m_codes) codes << e->text();
        o = Data::addBook(m_title->text(), m_author->text(), m_isbn->text(), codes);
        m_bookId = o.id;
    }
    if (!o.ok) {
        QMessageBox::warning(this, "Impossible d'enregistrer", o.error);
        return;   // on reste dans la fenêtre pour corriger
    }
    QDialog::accept();
}

// ================================================================ BookDialog
BookDialog::BookDialog(int bookId, QWidget *parent) : QDialog(parent), m_id(bookId)
{
    setModal(true);
    setMinimumSize(900, 680);
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_scroll = new QScrollArea;
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    root->addWidget(m_scroll, 1);

    auto *footer = new QFrame;
    footer->setObjectName("dialogFooter");
    auto *fl = new QHBoxLayout(footer);
    fl->setContentsMargins(24, 12, 24, 12);
    auto *edit = smallButton("Modifier le livre", "smallGhost");
    auto *del = smallButton("Supprimer le livre", "smallDanger");
    auto *close = new QPushButton("Fermer");
    close->setObjectName("primary");
    close->setCursor(Qt::PointingHandCursor);
    fl->addWidget(edit);
    fl->addWidget(del);
    fl->addStretch();
    fl->addWidget(close);
    root->addWidget(footer);
    connect(edit, &QPushButton::clicked, this, &BookDialog::editBook);
    connect(del, &QPushButton::clicked, this, &BookDialog::removeBook);
    connect(close, &QPushButton::clicked, this, &QDialog::accept);

    reload();
}

void BookDialog::reload()
{
    setWindowTitle("Fiche du livre — " + Data::book(m_id).title);
    // Remplace le contenu ; l'ancien est supprimé plus tard (on est peut-être dans le clic d'un de ses boutons).
    if (QWidget *old = m_scroll->takeWidget()) old->deleteLater();
    m_scroll->setWidget(build());
}

QWidget *BookDialog::build()
{
    const Book b = Data::book(m_id);
    const QList<Copy> copies = Data::copies(m_id);
    const QList<WaitEntry> queue = Data::waitingFor(m_id);
    const QString status = Data::statusOf(b);

    auto *w = new QFrame;
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(26, 24, 26, 18);
    v->setSpacing(14);

    // --- En-tête : couverture + informations
    auto *head = new QHBoxLayout;
    head->setSpacing(20);
    head->addWidget(new Cover(b.title, 96, 134), 0, Qt::AlignTop);
    auto *info = new QVBoxLayout;
    info->setSpacing(5);
    auto *title = new QLabel(b.title);
    title->setObjectName("dialogTitle");
    title->setWordWrap(true);
    info->addWidget(title);
    info->addWidget(Theme::muted("Auteur : " + b.author));
    info->addWidget(Theme::muted("ISBN : " + (b.isbn.isEmpty() ? QString("—") : b.isbn)));
    info->addWidget(Theme::muted("Ajouté le " + Data::fmt(b.added)));
    info->addSpacing(4);
    info->addWidget(Theme::badge(status), 0, Qt::AlignLeft);
    QString counts = b.copies == 0 ? "Aucun exemplaire enregistré"
                                   : QString("%1 / %2 exemplaires disponibles").arg(b.avail).arg(b.copies);
    if (b.waiting > 0) counts += QString("  •  %1 en liste d'attente").arg(b.waiting);
    info->addWidget(Theme::muted(counts));
    info->addStretch();
    head->addLayout(info, 1);
    v->addLayout(head);

    // --- Exemplaires
    auto *ch = new QHBoxLayout;
    auto *ct = new QLabel("Exemplaires");
    ct->setObjectName("cardTitle");
    ch->addWidget(ct);
    ch->addStretch();
    auto *add = smallButton("+ Ajouter un exemplaire", "smallPrimary");
    connect(add, &QPushButton::clicked, this, &BookDialog::addCopy);
    ch->addWidget(add);
    v->addLayout(ch);

    if (copies.isEmpty()) {
        v->addWidget(Theme::muted("Aucun exemplaire : ajoutez-en un (code-barres) pour pouvoir prêter ce livre."));
    } else {
        auto *t = new QTableWidget(copies.size(), 5);
        t->setHorizontalHeaderLabels({"Code-barres", "État", "Emprunté par", "Retour prévu", "Actions"});
        t->verticalHeader()->setVisible(false);
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->setSelectionMode(QAbstractItemView::NoSelection);
        t->setFocusPolicy(Qt::NoFocus);
        t->setShowGrid(false);
        t->setAlternatingRowColors(true);
        auto *hh = t->horizontalHeader();
        hh->setSectionResizeMode(0, QHeaderView::Stretch);
        hh->setSectionResizeMode(1, QHeaderView::Fixed);
        hh->setSectionResizeMode(2, QHeaderView::Stretch);
        hh->setSectionResizeMode(3, QHeaderView::Fixed);
        hh->setSectionResizeMode(4, QHeaderView::Fixed);
        t->setColumnWidth(1, 130);
        t->setColumnWidth(3, 120);
        t->setColumnWidth(4, 230);
        for (int r = 0; r < copies.size(); ++r) {
            const Copy &c = copies[r];
            t->setRowHeight(r, 48);
            t->setItem(r, 0, new QTableWidgetItem(c.barcode));
            auto *state = new QWidget;
            auto *sl = new QHBoxLayout(state);
            sl->setContentsMargins(8, 0, 8, 0);
            sl->addWidget(Theme::badge(c.state), 0, Qt::AlignLeft | Qt::AlignVCenter);
            t->setCellWidget(r, 1, state);
            t->setItem(r, 2, new QTableWidgetItem(c.borrower.isEmpty() ? "—" : c.borrower));
            t->setItem(r, 3, new QTableWidgetItem(c.state == "Emprunté" ? Data::fmt(c.due) : "—"));

            auto *actions = new QWidget;
            auto *al = new QHBoxLayout(actions);
            al->setContentsMargins(8, 0, 8, 0);
            al->setSpacing(6);
            const int id = c.id;
            if (c.state == "Disponible") {
                auto *lend = smallButton("Emprunter", "smallPrimary");
                auto *del = smallButton("Supprimer", "smallDanger");
                connect(lend, &QPushButton::clicked, this, [this, id] { lendCopy(id); });
                connect(del, &QPushButton::clicked, this, [this, id] { removeCopy(id); });
                al->addWidget(lend);
                al->addWidget(del);
            } else {
                auto *ret = smallButton("Rendre", "smallPrimary");
                connect(ret, &QPushButton::clicked, this, [this, id] { returnCopy(id); });
                al->addWidget(ret);
            }
            al->addStretch();
            t->setCellWidget(r, 4, actions);
        }
        t->setFixedHeight(qMin(copies.size(), 6) * 48 + 46);
        v->addWidget(t);
    }

    // --- Liste d'attente
    const bool canJoin = b.copies > 0 && b.avail - b.prio <= 0;
    if (!queue.isEmpty() || canJoin) {
        auto *wh = new QHBoxLayout;
        auto *wt = new QLabel(QString("Liste d'attente (%1)").arg(queue.size()));
        wt->setObjectName("cardTitle");
        wh->addWidget(wt);
        wh->addStretch();
        if (canJoin) {
            auto *join = smallButton("+ Rejoindre la liste d'attente", "smallPrimary");
            connect(join, &QPushButton::clicked, this, &BookDialog::joinWaitlist);
            wh->addWidget(join);
        }
        v->addLayout(wh);

        if (queue.isEmpty()) {
            v->addWidget(Theme::muted("Tous les exemplaires sont empruntés. Personne n'attend ce livre pour l'instant."));
        } else {
            auto *t = new QTableWidget(queue.size(), 5);
            t->setHorizontalHeaderLabels({"Rang", "Lecteur", "Demandé le", "Statut", "Action"});
            t->verticalHeader()->setVisible(false);
            t->setEditTriggers(QAbstractItemView::NoEditTriggers);
            t->setSelectionMode(QAbstractItemView::NoSelection);
            t->setFocusPolicy(Qt::NoFocus);
            t->setShowGrid(false);
            t->setAlternatingRowColors(true);
            auto *hh = t->horizontalHeader();
            hh->setSectionResizeMode(1, QHeaderView::Stretch);
            for (int c : {0, 2, 3, 4}) hh->setSectionResizeMode(c, QHeaderView::Fixed);
            t->setColumnWidth(0, 70);
            t->setColumnWidth(2, 120);
            t->setColumnWidth(3, 130);
            t->setColumnWidth(4, 130);
            for (int r = 0; r < queue.size(); ++r) {
                const WaitEntry &e = queue[r];
                t->setRowHeight(r, 46);
                t->setItem(r, 0, new QTableWidgetItem(QString::number(e.position)));
                t->setItem(r, 1, new QTableWidgetItem(e.reader));
                t->setItem(r, 2, new QTableWidgetItem(Data::fmt(e.date)));
                auto *state = new QWidget;
                auto *sl = new QHBoxLayout(state);
                sl->setContentsMargins(8, 0, 8, 0);
                sl->addWidget(Theme::badge(e.status == "Prioritaire" ? "Prioritaire" : "En attente"), 0,
                              Qt::AlignLeft | Qt::AlignVCenter);
                t->setCellWidget(r, 3, state);
                auto *act = new QWidget;
                auto *al = new QHBoxLayout(act);
                al->setContentsMargins(8, 0, 8, 0);
                auto *rm = smallButton("Retirer", "smallDanger");
                const int wid = e.id;
                connect(rm, &QPushButton::clicked, this, [this, wid] { removeWait(wid); });
                al->addWidget(rm);
                al->addStretch();
                t->setCellWidget(r, 4, act);
            }
            t->setFixedHeight(qMin(queue.size(), 5) * 46 + 46);
            v->addWidget(t);
        }
    }
    v->addStretch();
    return w;
}

bool BookDialog::pickReader(const QString &title, const QString &label, int *readerId)
{
    const QList<Reader> readers = Data::readers();
    if (readers.isEmpty()) {
        QMessageBox::information(this, "Aucun lecteur", "Ajoutez d'abord un lecteur dans la page « Lecteurs ».");
        return false;
    }
    QStringList items;
    for (const Reader &r : readers) items << QString("%1  (%2)").arg(r.name, r.matricule);
    bool ok = false;
    const QString chosen = QInputDialog::getItem(this, title, label, items, 0, false, &ok);
    if (!ok) return false;
    *readerId = readers.at(items.indexOf(chosen)).id;
    return true;
}

void BookDialog::showError(const Outcome &o)
{
    QMessageBox::warning(this, "Action impossible", o.error);
}

void BookDialog::editBook()
{
    AddBookDialog dlg(Data::book(m_id), this);
    if (dlg.exec() == QDialog::Accepted) {
        m_changed = true;
        reload();
    }
}

void BookDialog::removeBook()
{
    if (QMessageBox::question(this, "Supprimer le livre",
                              "Supprimer ce livre, ses exemplaires et sa liste d'attente ?") != QMessageBox::Yes)
        return;
    const Outcome o = Data::deleteBook(m_id);
    if (!o.ok) { showError(o); return; }
    m_changed = true;
    accept();
}

void BookDialog::addCopy()
{
    bool ok = false;
    const QString code = QInputDialog::getText(this, "Nouvel exemplaire", "Code-barres de l'exemplaire :",
                                               QLineEdit::Normal, QString(), &ok);
    if (!ok) return;
    const Outcome o = Data::addCopy(m_id, code);
    if (!o.ok) { showError(o); return; }
    m_changed = true;
    if (!o.promoted.isEmpty())
        QMessageBox::information(this, "Liste d'attente",
                                 o.promoted.join(", ") + " devient prioritaire pour ce nouvel exemplaire.");
    reload();
}

void BookDialog::lendCopy(int copyId)
{
    int readerId = 0;
    if (!pickReader("Emprunter", "Lecteur :", &readerId)) return;
    const Outcome o = Data::lend(copyId, readerId);
    if (!o.ok) { showError(o); return; }
    m_changed = true;
    reload();
}

void BookDialog::returnCopy(int copyId)
{
    const Outcome o = Data::giveBack(copyId);
    if (!o.ok) { showError(o); return; }
    m_changed = true;
    if (!o.promoted.isEmpty())
        QMessageBox::information(this, "Retour enregistré",
                                 "Exemplaire rendu.\n\n" + o.promoted.join(", ")
                                     + " est désormais prioritaire : cet exemplaire lui est réservé.");
    reload();
}

void BookDialog::removeCopy(int copyId)
{
    if (QMessageBox::question(this, "Supprimer l'exemplaire", "Supprimer cet exemplaire ?") != QMessageBox::Yes)
        return;
    const Outcome o = Data::deleteCopy(copyId);
    if (!o.ok) { showError(o); return; }
    m_changed = true;
    reload();
}

void BookDialog::joinWaitlist()
{
    int readerId = 0;
    if (!pickReader("Liste d'attente", "Lecteur à ajouter :", &readerId)) return;
    const Outcome o = Data::joinWaitlist(m_id, readerId);
    if (!o.ok) { showError(o); return; }
    m_changed = true;
    reload();
}

void BookDialog::removeWait(int waitId)
{
    const Outcome o = Data::removeWait(waitId);
    if (!o.ok) { showError(o); return; }
    m_changed = true;
    reload();
}
