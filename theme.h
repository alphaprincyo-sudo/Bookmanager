#pragma once
#include <QColor>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QStringList>

namespace Theme {
extern const QStringList serifFamilies;

QPixmap iconPixmap(const QString &name, const QColor &color, int size = 20);
QIcon icon(const QString &name, const QColor &color, int size = 20);
QLabel *iconLabel(const QString &name, const QColor &color, int size = 20);
QLabel *badge(const QString &text);   // pastille de statut colorée
QLabel *muted(const QString &text);   // texte secondaire grisé
}
