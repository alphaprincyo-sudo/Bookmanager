#include "theme.h"
#include <QByteArray>
#include <QFile>
#include <QHash>
#include <QPainter>
#include <QSvgRenderer>

namespace Theme {

const QStringList serifFamilies = {"Georgia", "Palatino Linotype", "Book Antiqua", "DejaVu Serif", "serif"};

QPixmap iconPixmap(const QString &name, const QColor &color, int size)
{
    QFile f(":/icons/" + name + ".svg");
    QByteArray svg;
    if (f.open(QIODevice::ReadOnly)){
        svg = f.readAll();
        svg.replace("#000000", color.name().toUtf8());   // les SVG utilisent #000000 comme couleur à remplacer
    }

    QSvgRenderer renderer(svg);
    QPixmap pm(size * 2, size * 2);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    renderer.render(&p);
    p.end();
    pm.setDevicePixelRatio(2);
    return pm;
}

QIcon icon(const QString &name, const QColor &color, int size)
{
    return QIcon(iconPixmap(name, color, size));
}

QLabel *iconLabel(const QString &name, const QColor &color, int size)
{
    auto *l = new QLabel;
    l->setPixmap(iconPixmap(name, color, size));
    l->setFixedSize(size, size);
    return l;
}

QLabel *badge(const QString &text)
{
    static const QHash<QString, QPair<QString, QString>> colors = {
        {"Disponible", {"#dcebd6", "#2c6a3f"}},
        {"Emprunté", {"#f5e5c6", "#8a5a10"}},
        {"Liste d'attente", {"#e5ddf3", "#5b4a9a"}},
        {"Indisponible", {"#f3e0d6", "#9a4a2a"}},
        {"En retard", {"#f8d9d2", "#b0412e"}},
        {"En cours", {"#d9e6f3", "#2f5f8f"}},
        {"Rendu", {"#e3e6dc", "#5b6b5b"}},
        {"Actif", {"#dcebd6", "#2c6a3f"}},
        {"Réservé", {"#e5ddf3", "#5b4a9a"}},
        {"En attente", {"#e5ddf3", "#5b4a9a"}},
        {"Prioritaire", {"#fbe3c8", "#a85a0a"}},
        {"Aucun exemplaire", {"#e3e6dc", "#5b6b5b"}},
    };
    const auto c = colors.value(text, {"#e3e6dc", "#5b6b5b"});
    auto *l = new QLabel(text);
    l->setAlignment(Qt::AlignCenter);
    l->setStyleSheet(QString("background:%1; color:%2; border-radius:10px; "
                             "padding:3px 11px; font-size:12px; font-weight:600;")
                         .arg(c.first, c.second));
    l->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    return l;
}

QLabel *muted(const QString &text)
{
    auto *l = new QLabel(text);
    l->setObjectName("muted");
    return l;
}

} // namespace Theme
