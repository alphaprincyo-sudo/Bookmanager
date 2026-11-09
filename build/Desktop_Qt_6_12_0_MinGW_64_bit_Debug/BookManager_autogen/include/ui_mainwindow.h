/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include "widgets.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QFrame *centralwidget;
    QHBoxLayout *rootLayout;
    SidebarFrame *sidebar;
    QVBoxLayout *sidebarLayout;
    QSpacerItem *logoSpacer;
    QPushButton *btnAccueil;
    QPushButton *btnLivres;
    QPushButton *btnLecteurs;
    QPushButton *btnEmprunts;
    QPushButton *btnAttente;
    QPushButton *btnParametres;
    QSpacerItem *sidebarSpacer;
    QVBoxLayout *rightLayout;
    QFrame *topbar;
    QHBoxLayout *topbarLayout;
    QLineEdit *searchEdit;
    QSpacerItem *topSpacer;
    QVBoxLayout *userLayout;
    QStackedWidget *stack;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1400, 880);
        centralwidget = new QFrame(MainWindow);
        centralwidget->setObjectName("centralwidget");
        rootLayout = new QHBoxLayout(centralwidget);
        rootLayout->setSpacing(0);
        rootLayout->setObjectName("rootLayout");
        rootLayout->setContentsMargins(0, 0, 0, 0);
        sidebar = new SidebarFrame(centralwidget);
        sidebar->setObjectName("sidebar");
        sidebar->setMinimumSize(QSize(260, 0));
        sidebar->setMaximumSize(QSize(260, 16777215));
        sidebarLayout = new QVBoxLayout(sidebar);
        sidebarLayout->setSpacing(6);
        sidebarLayout->setObjectName("sidebarLayout");
        sidebarLayout->setContentsMargins(18, 0, 18, 26);
        logoSpacer = new QSpacerItem(20, 150, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);

        sidebarLayout->addItem(logoSpacer);

        btnAccueil = new QPushButton(sidebar);
        btnAccueil->setObjectName("btnAccueil");
        btnAccueil->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnAccueil->setIconSize(QSize(22, 22));
        btnAccueil->setCheckable(true);
        btnAccueil->setChecked(true);
        btnAccueil->setAutoExclusive(true);

        sidebarLayout->addWidget(btnAccueil);

        btnLivres = new QPushButton(sidebar);
        btnLivres->setObjectName("btnLivres");
        btnLivres->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnLivres->setIconSize(QSize(22, 22));
        btnLivres->setCheckable(true);
        btnLivres->setChecked(false);
        btnLivres->setAutoExclusive(true);

        sidebarLayout->addWidget(btnLivres);

        btnLecteurs = new QPushButton(sidebar);
        btnLecteurs->setObjectName("btnLecteurs");
        btnLecteurs->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnLecteurs->setIconSize(QSize(22, 22));
        btnLecteurs->setCheckable(true);
        btnLecteurs->setChecked(false);
        btnLecteurs->setAutoExclusive(true);

        sidebarLayout->addWidget(btnLecteurs);

        btnEmprunts = new QPushButton(sidebar);
        btnEmprunts->setObjectName("btnEmprunts");
        btnEmprunts->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnEmprunts->setIconSize(QSize(22, 22));
        btnEmprunts->setCheckable(true);
        btnEmprunts->setChecked(false);
        btnEmprunts->setAutoExclusive(true);

        sidebarLayout->addWidget(btnEmprunts);

        btnAttente = new QPushButton(sidebar);
        btnAttente->setObjectName("btnAttente");
        btnAttente->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnAttente->setIconSize(QSize(22, 22));
        btnAttente->setCheckable(true);
        btnAttente->setChecked(false);
        btnAttente->setAutoExclusive(true);

        sidebarLayout->addWidget(btnAttente);

        btnParametres = new QPushButton(sidebar);
        btnParametres->setObjectName("btnParametres");
        btnParametres->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnParametres->setIconSize(QSize(22, 22));
        btnParametres->setCheckable(true);
        btnParametres->setChecked(false);
        btnParametres->setAutoExclusive(true);

        sidebarLayout->addWidget(btnParametres);

        sidebarSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        sidebarLayout->addItem(sidebarSpacer);


        rootLayout->addWidget(sidebar);

        rightLayout = new QVBoxLayout();
        rightLayout->setSpacing(0);
        rightLayout->setObjectName("rightLayout");
        rightLayout->setContentsMargins(0, 0, 0, 0);
        topbar = new QFrame(centralwidget);
        topbar->setObjectName("topbar");
        topbar->setMinimumSize(QSize(0, 84));
        topbar->setMaximumSize(QSize(16777215, 84));
        topbarLayout = new QHBoxLayout(topbar);
        topbarLayout->setObjectName("topbarLayout");
        topbarLayout->setContentsMargins(26, 0, 26, 0);
        searchEdit = new QLineEdit(topbar);
        searchEdit->setObjectName("searchEdit");
        searchEdit->setMinimumSize(QSize(520, 0));

        topbarLayout->addWidget(searchEdit);

        topSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        topbarLayout->addItem(topSpacer);

        userLayout = new QVBoxLayout();
        userLayout->setSpacing(0);
        userLayout->setObjectName("userLayout");

        topbarLayout->addLayout(userLayout);


        rightLayout->addWidget(topbar);

        stack = new QStackedWidget(centralwidget);
        stack->setObjectName("stack");

        rightLayout->addWidget(stack);


        rootLayout->addLayout(rightLayout);

        rootLayout->setStretch(1, 1);
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Book Manager \342\200\223 Biblioth\303\250que universitaire", nullptr));
        btnAccueil->setText(QCoreApplication::translate("MainWindow", "  Accueil", nullptr));
        btnLivres->setText(QCoreApplication::translate("MainWindow", "  Livres", nullptr));
        btnLecteurs->setText(QCoreApplication::translate("MainWindow", "  Lecteurs", nullptr));
        btnEmprunts->setText(QCoreApplication::translate("MainWindow", "  Emprunts", nullptr));
        btnAttente->setText(QCoreApplication::translate("MainWindow", "  Liste d'attente", nullptr));
        btnParametres->setText(QCoreApplication::translate("MainWindow", "  Param\303\250tres", nullptr));
        searchEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Rechercher un livre, un lecteur, un titre...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
