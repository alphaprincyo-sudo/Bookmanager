#pragma once
#include <QFrame>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
#include "data.h"

// Couverture par défaut : fond coloré avec uniquement le titre du livre.
class Cover : public QWidget
{
    Q_OBJECT
public:
    explicit Cover(const QString &title, int w = 48, int h = 66, QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QString m_title;
};

// Carte avec bandeau de titre (icône + titre + bouton d'action ou widget à droite).
class SectionCard : public QFrame
{
    Q_OBJECT
public:
    SectionCard(const QString &title, const QString &iconName,
                const QString &actionText = QString(), QWidget *right = nullptr,
                QWidget *parent = nullptr);
    QVBoxLayout *body() const { return m_body; }
signals:
    void actionClicked();
private:
    QVBoxLayout *m_body;
};

// Ligne de livre (couverture, titre, auteur, infos, pastille de statut).
class BookRow : public QFrame
{
    Q_OBJECT
public:
    BookRow(const Book &b, const QString &line3 = QString(), const QString &badgeText = QString(),
            int rank = 0, QWidget *parent = nullptr);
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent *) override { emit clicked(); }
};

// Carte de statistique cliquable.
class StatCard : public QFrame
{
    Q_OBJECT
public:
    StatCard(const QString &iconName, const QString &color, const QString &label,
             const QString &value, const QString &sub, QWidget *parent = nullptr);
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent *) override { emit clicked(); }
};

// Barre latérale : l'image mimi.png est dessinée à la largeur de la fenêtre sans déformer le logo ;
// seule la zone unie du milieu s'étire quand la fenêtre change de hauteur.
class SidebarFrame : public QFrame
{
    Q_OBJECT
public:
    explicit SidebarFrame(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QPixmap m_pm;
};

// Bandeau d'accueil : image (banner_card.png) dessinée en « cover », coins arrondis.
// Les widgets enfants (textes) se placent par-dessus via un layout.
class ImageBanner : public QFrame
{
    Q_OBJECT
public:
    explicit ImageBanner(const QString &resource, QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QPixmap m_pm;
};

// Carte d'un livre dans le catalogue ; double-clic = ouvrir la fiche (modal).
class BookCard : public QFrame
{
    Q_OBJECT
public:
    explicit BookCard(const Book &b, QWidget *parent = nullptr);
signals:
    void doubleClicked(int bookId);
protected:
    void mouseDoubleClickEvent(QMouseEvent *) override { emit doubleClicked(m_id); }
private:
    int m_id;
};

// Graphique « Activité récente » (emprunts / retours).
class ActivityChart : public QWidget
{
    Q_OBJECT
public:
    explicit ActivityChart(QWidget *parent = nullptr);
    void setSeries(const QList<int> &emprunts, const QList<int> &retours);
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QList<int> m_emp, m_ret;
};

void clearLayout(QLayout *layout);
QTableWidget *makeTable(const QStringList &headers, const QList<QStringList> &rows, int statusCol = -1);
// Page avec titre + sous-titre dans une zone défilante ; `content` reçoit le layout à remplir.
QScrollArea *pageShell(const QString &title, const QString &subtitle, QVBoxLayout *&content);
