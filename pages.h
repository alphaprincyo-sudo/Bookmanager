#pragma once
#include <QGridLayout>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

namespace Ui { class ParametresPage; }

// Index des pages dans le QStackedWidget (même ordre que le menu)
enum Page { PageAccueil, PageLivres, PageLecteurs, PageEmprunts, PageAttente, PageParametres };

// Accueil : bandeau, statistiques, activité, derniers ajouts, populaires, plus attendus.
class HomePage : public QScrollArea
{
    Q_OBJECT
public:
    explicit HomePage(QWidget *parent = nullptr);
    void refresh();
signals:
    void navigate(int pageIndex);
private:
    int m_days = 7;
};

// Catalogue : filtres par statut, recherche, ajout ; double-clic sur un livre = fiche (modal).
class LivresPage : public QWidget
{
    Q_OBJECT
public:
    explicit LivresPage(QWidget *parent = nullptr);
    void setQuery(const QString &text);
    void refresh();
signals:
    void dataChanged();
private:
    void openBook(int bookId);
    void addBook();
    QGridLayout *m_grid;
    QString m_query;
    QString m_filter = "Tous";
};

// Lecteurs inscrits.
class LecteursPage : public QWidget
{
    Q_OBJECT
public:
    explicit LecteursPage(QWidget *parent = nullptr);
    void refresh();
signals:
    void dataChanged();
private:
    void addReader();
    QVBoxLayout *m_cardLayout;
};

// Prêts en cours, retards, retours.
class EmpruntsPage : public QWidget
{
    Q_OBJECT
public:
    explicit EmpruntsPage(QWidget *parent = nullptr);
    void refresh();
signals:
    void dataChanged();
private:
    QVBoxLayout *m_cardLayout;
};

// Livres possédés mais indisponibles, avec leur file d'attente.
class ListeAttentePage : public QWidget
{
    Q_OBJECT
public:
    explicit ListeAttentePage(QWidget *parent = nullptr);
    void refresh();
signals:
    void dataChanged();
private:
    QVBoxLayout *m_content;
};

// Paramètres : interface définie dans parametrespage.ui (Qt Designer).
class ParametresPage : public QWidget
{
    Q_OBJECT
public:
    explicit ParametresPage(QWidget *parent = nullptr);
    ~ParametresPage();
signals:
    void dataChanged();
private:
    Ui::ParametresPage *ui;
};
