#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "data.h"
#include "pages.h"
#include "theme.h"
#include <QCompleter>
#include <QIcon>
#include <QStringListModel>
#include "widgets.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);   // interface dessinée dans mainwindow.ui
    setWindowIcon(QIcon(":/icons/gra.png"));
    resize(1400, 880);
    setMinimumSize(1100, 700);

    // Menu (l'image de fond de la barre latérale vient de style.qss : icons/mimi.png)
    m_navButtons = {ui->btnAccueil, ui->btnLivres, ui->btnLecteurs,
                    ui->btnEmprunts, ui->btnAttente, ui->btnParametres};
    const QStringList icons = {"home", "book", "users", "swap", "queue", "settings"};
    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setIcon(Theme::icon(icons[i], QColor("#eef0e2"), 22));
        connect(m_navButtons[i], &QPushButton::clicked, this, [this, i] { goTo(i); });
    }

    // Barre du haut
    ui->searchEdit->addAction(Theme::icon("search", QColor("#4a5a4a"), 20), QLineEdit::LeadingPosition);
    //ui->btnBell->setIcon(Theme::icon("bell", QColor("#2f6a4a"), 24));
    //ui->avatarLabel->setPixmap(Theme::iconPixmap("user", QColor("#f4f1e4"), 20));
    //ui->chevronLabel->setPixmap(Theme::iconPixmap("chevdown", QColor("#4a5a4a"), 16));
    connect(ui->searchEdit, &QLineEdit::returnPressed, this, &MainWindow::onSearch);

    buildPages();
    reloadCompleter();
    goTo(PageAccueil);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::buildPages()
{
    m_home = new HomePage;
    m_livres = new LivresPage;
    m_lecteurs = new LecteursPage;
    m_emprunts = new EmpruntsPage;
    m_attente = new ListeAttentePage;
    auto *params = new ParametresPage;

    connect(m_home, &HomePage::navigate, this, &MainWindow::goTo);
    // Toute modification de données : on met à jour l'auto-complétion (les pages se rechargent à l'ouverture)
    for (QObject *p : {static_cast<QObject *>(m_livres), static_cast<QObject *>(m_lecteurs),
                       static_cast<QObject *>(m_emprunts), static_cast<QObject *>(m_attente),
                       static_cast<QObject *>(params)})
        connect(p, SIGNAL(dataChanged()), this, SLOT(reloadCompleter()));

    // Même ordre que l'enum Page et que les boutons du menu
    ui->stack->addWidget(m_home);
    ui->stack->addWidget(m_livres);
    ui->stack->addWidget(m_lecteurs);
    ui->stack->addWidget(m_emprunts);
    ui->stack->addWidget(m_attente);
    ui->stack->addWidget(params);
}

void MainWindow::reloadCompleter()
{
    QStringList words;
    for (const Book &b : Data::books()) words << b.title << b.author;
    words.removeDuplicates();
    auto *completer = new QCompleter(new QStringListModel(words), this);
    completer->model()->setParent(completer);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    if (QCompleter *old = ui->searchEdit->completer()) old->deleteLater();
    ui->searchEdit->setCompleter(completer);
}

void MainWindow::goTo(int pageIndex)
{
    if (pageIndex < 0 || pageIndex >= m_navButtons.size() || pageIndex >= ui->stack->count()) return;
    m_navButtons.at(pageIndex)->setChecked(true);
    ui->stack->setCurrentIndex(pageIndex);

    switch (pageIndex) {   // les données ont pu changer : on recharge la page affichée
    case PageAccueil: m_home->refresh(); break;
    case PageLivres: m_livres->refresh(); break;
    case PageLecteurs: m_lecteurs->refresh(); break;
    case PageEmprunts: m_emprunts->refresh(); break;
    case PageAttente: m_attente->refresh(); break;
    default: break;
    }
}

void MainWindow::onSearch()
{
    m_livres->setQuery(ui->searchEdit->text());   // champ vide = on retire le filtre
    goTo(PageLivres);
}
