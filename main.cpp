#include <QApplication>
#include <QFile>
#include <QFont>
#include <QMessageBox>
#include "data.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("BookManager");
    QCoreApplication::setApplicationName("BookManager");

    QFont font;
    font.setFamilies({"Segoe UI", "Inter", "Noto Sans", "DejaVu Sans"});
    font.setPixelSize(14);
    app.setFont(font);

    QFile qss(":/style.qss");
    if (qss.open(QIODevice::ReadOnly))
        app.setStyleSheet(QString::fromUtf8(qss.readAll()));

    QString error;
    if (!Data::open("bookmanager.db", &error)) {   // créé dans le dossier d'exécution
        QMessageBox::critical(nullptr, "Base de données", error);
        return 1;
    }

    MainWindow w;
    w.show();
    return app.exec();
}
