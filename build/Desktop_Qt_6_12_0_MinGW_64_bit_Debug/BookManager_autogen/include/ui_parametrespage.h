/********************************************************************************
** Form generated from reading UI file 'parametrespage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARAMETRESPAGE_H
#define UI_PARAMETRESPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ParametresPage
{
public:
    QVBoxLayout *pageLayout;
    QLabel *pageTitle;
    QLabel *pageSubtitle;
    QFrame *card;
    QVBoxLayout *cardLayout;
    QLabel *cardTitle;
    QFormLayout *formLayout;
    QLabel *lblNom;
    QLineEdit *nameEdit;
    QLabel *lblDuree;
    QSpinBox *dureeSpin;
    QLabel *lblMax;
    QSpinBox *maxSpin;
    QLabel *lblLangue;
    QComboBox *langCombo;
    QCheckBox *chkAttente;
    QCheckBox *chkRappel;
    QPushButton *btnEnregistrer;
    QPushButton *btnDemo;
    QSpacerItem *pageSpacer;

    void setupUi(QWidget *ParametresPage)
    {
        if (ParametresPage->objectName().isEmpty())
            ParametresPage->setObjectName("ParametresPage");
        ParametresPage->resize(900, 600);
        pageLayout = new QVBoxLayout(ParametresPage);
        pageLayout->setSpacing(16);
        pageLayout->setObjectName("pageLayout");
        pageLayout->setContentsMargins(28, 24, 28, 28);
        pageTitle = new QLabel(ParametresPage);
        pageTitle->setObjectName("pageTitle");

        pageLayout->addWidget(pageTitle);

        pageSubtitle = new QLabel(ParametresPage);
        pageSubtitle->setObjectName("pageSubtitle");

        pageLayout->addWidget(pageSubtitle);

        card = new QFrame(ParametresPage);
        card->setObjectName("card");
        cardLayout = new QVBoxLayout(card);
        cardLayout->setSpacing(14);
        cardLayout->setObjectName("cardLayout");
        cardLayout->setContentsMargins(22, 18, 22, 20);
        cardTitle = new QLabel(card);
        cardTitle->setObjectName("cardTitle");

        cardLayout->addWidget(cardTitle);

        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        formLayout->setHorizontalSpacing(20);
        formLayout->setVerticalSpacing(14);
        lblNom = new QLabel(card);
        lblNom->setObjectName("lblNom");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, lblNom);

        nameEdit = new QLineEdit(card);
        nameEdit->setObjectName("nameEdit");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, nameEdit);

        lblDuree = new QLabel(card);
        lblDuree->setObjectName("lblDuree");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, lblDuree);

        dureeSpin = new QSpinBox(card);
        dureeSpin->setObjectName("dureeSpin");
        dureeSpin->setMinimum(1);
        dureeSpin->setMaximum(90);
        dureeSpin->setValue(14);

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, dureeSpin);

        lblMax = new QLabel(card);
        lblMax->setObjectName("lblMax");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, lblMax);

        maxSpin = new QSpinBox(card);
        maxSpin->setObjectName("maxSpin");
        maxSpin->setMinimum(1);
        maxSpin->setMaximum(20);
        maxSpin->setValue(5);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, maxSpin);

        lblLangue = new QLabel(card);
        lblLangue->setObjectName("lblLangue");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, lblLangue);

        langCombo = new QComboBox(card);
        langCombo->addItem(QString());
        langCombo->addItem(QString());
        langCombo->addItem(QString());
        langCombo->setObjectName("langCombo");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, langCombo);


        cardLayout->addLayout(formLayout);

        chkAttente = new QCheckBox(card);
        chkAttente->setObjectName("chkAttente");
        chkAttente->setChecked(true);

        cardLayout->addWidget(chkAttente);

        chkRappel = new QCheckBox(card);
        chkRappel->setObjectName("chkRappel");
        chkRappel->setChecked(true);

        cardLayout->addWidget(chkRappel);

        btnEnregistrer = new QPushButton(card);
        btnEnregistrer->setObjectName("btnEnregistrer");
        btnEnregistrer->setMaximumSize(QSize(180, 16777215));
        btnEnregistrer->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        cardLayout->addWidget(btnEnregistrer);

        btnDemo = new QPushButton(card);
        btnDemo->setObjectName("btnDemo");
        btnDemo->setMaximumSize(QSize(320, 16777215));
        btnDemo->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        cardLayout->addWidget(btnDemo);


        pageLayout->addWidget(card);

        pageSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        pageLayout->addItem(pageSpacer);


        retranslateUi(ParametresPage);

        QMetaObject::connectSlotsByName(ParametresPage);
    } // setupUi

    void retranslateUi(QWidget *ParametresPage)
    {
        pageTitle->setText(QCoreApplication::translate("ParametresPage", "Param\303\250tres", nullptr));
        pageSubtitle->setText(QCoreApplication::translate("ParametresPage", "Configuration g\303\251n\303\251rale de la biblioth\303\250que.", nullptr));
        cardTitle->setText(QCoreApplication::translate("ParametresPage", "R\303\250gles de pr\303\252t", nullptr));
        lblNom->setText(QCoreApplication::translate("ParametresPage", "Nom de la biblioth\303\250que", nullptr));
        nameEdit->setText(QCoreApplication::translate("ParametresPage", "Biblioth\303\250que universitaire", nullptr));
        lblDuree->setText(QCoreApplication::translate("ParametresPage", "Dur\303\251e de pr\303\252t", nullptr));
        dureeSpin->setSuffix(QCoreApplication::translate("ParametresPage", " jours", nullptr));
        lblMax->setText(QCoreApplication::translate("ParametresPage", "Livres maximum par lecteur", nullptr));
        lblLangue->setText(QCoreApplication::translate("ParametresPage", "Langue", nullptr));
        langCombo->setItemText(0, QCoreApplication::translate("ParametresPage", "Fran\303\247ais", nullptr));
        langCombo->setItemText(1, QCoreApplication::translate("ParametresPage", "Malagasy", nullptr));
        langCombo->setItemText(2, QCoreApplication::translate("ParametresPage", "English", nullptr));

        chkAttente->setText(QCoreApplication::translate("ParametresPage", "Pr\303\251venir automatiquement le premier de la liste d'attente au retour d'un livre", nullptr));
        chkRappel->setText(QCoreApplication::translate("ParametresPage", "Envoyer un rappel avant la date de retour", nullptr));
        btnEnregistrer->setText(QCoreApplication::translate("ParametresPage", "Enregistrer", nullptr));
        btnDemo->setText(QCoreApplication::translate("ParametresPage", "Charger des donn\303\251es de d\303\251monstration", nullptr));
        (void)ParametresPage;
    } // retranslateUi

};

namespace Ui {
    class ParametresPage: public Ui_ParametresPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARAMETRESPAGE_H
