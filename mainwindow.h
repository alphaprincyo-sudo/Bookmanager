#pragma once
#include <QList>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QPushButton;
class HomePage;
class LivresPage;
class LecteursPage;
class EmpruntsPage;
class ListeAttentePage;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void goTo(int pageIndex);      // change de page, met à jour le menu et recharge la page

private slots:
    void onSearch();
    void reloadCompleter();        // auto-complétion titres/auteurs

private:
    void buildPages();

    Ui::MainWindow *ui;
    QList<QPushButton *> m_navButtons;
    HomePage *m_home = nullptr;
    LivresPage *m_livres = nullptr;
    LecteursPage *m_lecteurs = nullptr;
    EmpruntsPage *m_emprunts = nullptr;
    ListeAttentePage *m_attente = nullptr;
};
