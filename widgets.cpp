#include "widgets.h"
#include "theme.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QTextOption>

// ---------------------------------------------------------------- Cover
Cover::Cover(const QString &title, int w, int h, QWidget *parent)
    : QWidget(parent), m_title(title)
{
    setFixedSize(w, h);
}

void Cover::paintEvent(QPaintEvent *)
{
    static const QStringList colors = {"#2f5d46", "#3d5a80", "#7a4e3a", "#5b4a7a",
                                       "#8a6d3b", "#3b6b6b", "#6b3d4a"};
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QColor base(colors.at(qChecksum(m_title.toUtf8()) % colors.size()));
    const QRectF r(0.5, 0.5, width() - 1, height() - 1);

    QLinearGradient g(r.topLeft(), r.bottomRight());
    g.setColorAt(0, base.lighter(118));
    g.setColorAt(1, base.darker(130));
    p.setBrush(g);
    p.setPen(QPen(QColor(0, 0, 0, 50), 1));
    p.drawRoundedRect(r, 4, 4);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 45));
    p.drawRect(QRectF(4, 1, 2, height() - 2));   // tranche du livre

    QFont f;
    f.setFamilies(Theme::serifFamilies);
    f.setBold(true);
    f.setPixelSize(qMax(7, int(width() * 0.115)));
    p.setFont(f);
    p.setPen(QColor("#f4f1e4"));
    QTextOption opt(Qt::AlignCenter);
    opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    p.drawText(r.adjusted(8, 6, -4, -6), m_title, opt);
}

// ---------------------------------------------------------------- SectionCard
SectionCard::SectionCard(const QString &title, const QString &iconName, const QString &actionText,
                         QWidget *right, QWidget *parent)
    : QFrame(parent)
{
    setObjectName("card");
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto *head = new QFrame;
    head->setObjectName("cardHead");
    auto *h = new QHBoxLayout(head);
    h->setContentsMargins(18, 12, 14, 12);
    h->setSpacing(10);
    h->addWidget(Theme::iconLabel(iconName, QColor("#2f6a4a"), 22));
    auto *t = new QLabel(title);
    t->setObjectName("cardTitle");
    h->addWidget(t);
    h->addStretch();
    if (right) {
        h->addWidget(right);
    } else if (!actionText.isEmpty()) {
        auto *b = new QPushButton(actionText);
        b->setObjectName("link");
        b->setCursor(Qt::PointingHandCursor);
        connect(b, &QPushButton::clicked, this, &SectionCard::actionClicked);
        h->addWidget(b);
    }
    outer->addWidget(head);

    m_body = new QVBoxLayout;
    m_body->setContentsMargins(18, 8, 18, 12);
    m_body->setSpacing(0);
    auto *wrap = new QFrame;
    wrap->setLayout(m_body);
    outer->addWidget(wrap);
}

// ---------------------------------------------------------------- BookRow
BookRow::BookRow(const Book &b, const QString &line3, const QString &badgeText, int rank, QWidget *parent)
    : QFrame(parent)
{
    setObjectName("row");
    setCursor(Qt::PointingHandCursor);
    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 8, 0, 8);
    lay->setSpacing(12);
    if (rank > 0) {
        auto *r = new QLabel(QString::number(rank));
        r->setObjectName("rank");
        lay->addWidget(r);
    }
    lay->addWidget(new Cover(b.title, 46, 62));

    auto *col = new QVBoxLayout;
    col->setSpacing(2);
    auto *t = new QLabel(b.title);
    t->setObjectName("rowTitle");
    t->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    col->addWidget(t);
    for (const QString &txt : {b.author, line3}) {
        if (txt.isEmpty()) continue;
        auto *m = Theme::muted(txt);
        m->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        col->addWidget(m);
    }
    lay->addLayout(col, 1);
    if (!badgeText.isEmpty())
        lay->addWidget(Theme::badge(badgeText), 0, Qt::AlignVCenter);
    lay->addWidget(Theme::iconLabel("chevron", QColor("#5b6b5b"), 16));
}

// ---------------------------------------------------------------- StatCard
StatCard::StatCard(const QString &iconName, const QString &color, const QString &label,
                   const QString &value, const QString &sub, QWidget *parent)
    : QFrame(parent)
{
    setObjectName("card");
    setCursor(Qt::PointingHandCursor);
    setMinimumWidth(120);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(18, 18, 14, 12);
    lay->setSpacing(2);

    auto *circle = new QLabel;
    circle->setFixedSize(54, 54);
    circle->setAlignment(Qt::AlignCenter);
    circle->setPixmap(Theme::iconPixmap(iconName, Qt::white, 28));
    circle->setStyleSheet(QString("background:%1; border-radius:27px;").arg(color));
    lay->addWidget(circle);
    lay->addSpacing(8);

    auto *l = new QLabel(label);
    l->setObjectName("statLabel");
    l->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    lay->addWidget(l);
    auto *v = new QLabel(value);
    v->setObjectName("statValue");
    lay->addWidget(v);

    auto *bottom = new QHBoxLayout;
    auto *s = Theme::muted(sub);
    s->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    bottom->addWidget(s, 1);
    bottom->addWidget(Theme::iconLabel("leaf", QColor("#a9c4a2"), 26));
    lay->addLayout(bottom);
}

// ---------------------------------------------------------------- SidebarFrame
SidebarFrame::SidebarFrame(QWidget *parent) : QFrame(parent), m_pm(":/icons/mimi.png") {}

void SidebarFrame::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHints(QPainter::SmoothPixmapTransform);
    p.fillRect(rect(), QColor("#1f4a35"));
    if (m_pm.isNull()) return;
    // Découpe de l'image : haut (logo + titre) | milieu uni | bas (feuilles + devise)
    const double top = 420, bottom = 1000;
    const double s = double(width()) / m_pm.width();
    const double topH = top * s, botH = (m_pm.height() - bottom) * s;
    const double midH = qMax(0.0, height() - topH - botH);
    p.drawPixmap(QRectF(0, 0, width(), topH), m_pm, QRectF(0, 0, m_pm.width(), top));
    p.drawPixmap(QRectF(0, topH, width(), midH), m_pm, QRectF(0, top, m_pm.width(), bottom - top));
    p.drawPixmap(QRectF(0, topH + midH, width(), botH), m_pm, QRectF(0, bottom, m_pm.width(), m_pm.height() - bottom));
}

// ---------------------------------------------------------------- ImageBanner
ImageBanner::ImageBanner(const QString &resource, QWidget *parent) : QFrame(parent), m_pm(resource) {}

void ImageBanner::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath clip;
    clip.addRoundedRect(QRectF(rect()), 14, 14);
    p.setClipPath(clip);
    p.fillRect(rect(), QColor("#dfe7d3"));
    if (m_pm.isNull()) return;
    // « cover » : l'image remplit tout le bandeau, alignée à droite (là où sont les livres)
    const double s = qMax(double(width()) / m_pm.width(), double(height()) / m_pm.height());
    const double w = m_pm.width() * s, h = m_pm.height() * s;
    p.drawPixmap(QRectF(width() - w, (height() - h) / 2, w, h), m_pm, QRectF(m_pm.rect()));
}

// ---------------------------------------------------------------- BookCard
BookCard::BookCard(const Book &b, QWidget *parent) : QFrame(parent), m_id(b.id)
{
    setObjectName("bookCard");
    setCursor(Qt::PointingHandCursor);
    setToolTip("Double-cliquez pour voir les exemplaires");
    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(16, 14, 16, 14);
    lay->setSpacing(16);
    lay->addWidget(new Cover(b.title, 72, 100), 0, Qt::AlignTop);

    auto *col = new QVBoxLayout;
    col->setSpacing(3);
    auto *t = new QLabel(b.title);
    t->setObjectName("rowTitle");
    t->setWordWrap(true);
    col->addWidget(t);
    col->addWidget(Theme::muted(b.author));
    col->addWidget(Theme::muted(b.copies == 0
                                    ? QString("Aucun exemplaire enregistré")
                                    : QString("%1 / %2 exemplaires disponibles").arg(b.avail).arg(b.copies)));
    if (b.waiting > 0)
        col->addWidget(Theme::muted(QString("%1 en liste d'attente").arg(b.waiting)));
    else if (b.due.isValid() && b.avail == 0)
        col->addWidget(Theme::muted("Retour prévu le " + Data::fmt(b.due)));
    col->addSpacing(4);
    col->addWidget(Theme::badge(Data::statusOf(b)), 0, Qt::AlignLeft);
    col->addStretch();
    lay->addLayout(col, 1);
}

// ---------------------------------------------------------------- ActivityChart
ActivityChart::ActivityChart(QWidget *parent) : QWidget(parent) { setMinimumHeight(270); }

void ActivityChart::setSeries(const QList<int> &emprunts, const QList<int> &retours)
{
    m_emp = emprunts;
    m_ret = retours;
    update();
}

static QPainterPath smoothPath(const QList<QPointF> &pts)
{
    QPainterPath path(pts.first());
    for (int i = 1; i < pts.size(); ++i) {
        const double mx = (pts[i - 1].x() + pts[i].x()) / 2;
        path.cubicTo(QPointF(mx, pts[i - 1].y()), QPointF(mx, pts[i].y()), pts[i]);
    }
    return path;
}

void ActivityChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int n = m_emp.size();
    if (n < 2 || m_ret.size() != n) return;

    const double W = width(), H = height();
    const double left = 40, right = 22, top = 38, bottom = 30;
    const QRectF plot(left, top, W - left - right, H - top - bottom);

    int maxV = 10;
    for (int v : m_emp) maxV = qMax(maxV, v);
    for (int v : m_ret) maxV = qMax(maxV, v);
    const int ymax = ((maxV + 9) / 10) * 10;   // arrondi à la dizaine supérieure

    QFont f;
    f.setPixelSize(12);
    p.setFont(f);
    for (int i = 0; i <= 5; ++i) {
        const int v = ymax * i / 5;
        const double y = plot.bottom() - plot.height() * i / 5;
        p.setPen(QPen(QColor("#e3e1d2"), 1));
        p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        p.setPen(QColor("#5f6b5f"));
        p.drawText(QRectF(0, y - 9, left - 10, 18), Qt::AlignRight | Qt::AlignVCenter, QString::number(v));
    }

    struct Series { QList<int> data; QColor col; int alpha; };
    const QList<Series> series = {{m_ret, QColor("#8fb68e"), 90}, {m_emp, QColor("#2f6a4a"), 110}};
    QList<double> xs;
    for (int i = 0; i < n; ++i)
        xs << plot.left() + 8 + (plot.width() - 16) * i / (n - 1);
    auto points = [&](const Series &s) {
        QList<QPointF> pts;
        for (int i = 0; i < n; ++i)
            pts << QPointF(xs[i], plot.bottom() - plot.height() * s.data[i] / ymax);
        return pts;
    };

    for (const Series &s : series) {   // aires
        QPainterPath fill = smoothPath(points(s));
        fill.lineTo(QPointF(xs.last(), plot.bottom()));
        fill.lineTo(QPointF(xs.first(), plot.bottom()));
        fill.closeSubpath();
        QLinearGradient g(0, plot.top(), 0, plot.bottom());
        QColor c1 = s.col, c2 = s.col;
        c1.setAlpha(s.alpha);
        c2.setAlpha(10);
        g.setColorAt(0, c1);
        g.setColorAt(1, c2);
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        p.drawPath(fill);
    }
    for (const Series &s : series) {   // courbes + points
        const auto pts = points(s);
        p.setPen(QPen(s.col, 2.4));
        p.setBrush(Qt::NoBrush);
        p.drawPath(smoothPath(pts));
        p.setBrush(s.col);
        p.setPen(QPen(QColor("#f7f5ec"), 1.5));
        for (const QPointF &pt : pts) p.drawEllipse(pt, 4, 4);
    }

    p.setPen(QColor("#5f6b5f"));   // dates
    for (int i = 0; i < n; ++i) {
        const QDate d = QDate::currentDate().addDays(-(n - 1 - i));
        p.drawText(QRectF(xs[i] - 24, plot.bottom() + 8, 48, 18), Qt::AlignCenter, d.toString("dd/MM"));
    }

    double lx = W - right - 175;   // légende
    for (const auto &item : {QPair<QString, QColor>{"Emprunts", QColor("#2f6a4a")},
                             QPair<QString, QColor>{"Retours", QColor("#8fb68e")}}) {
        p.setPen(Qt::NoPen);
        p.setBrush(item.second);
        p.drawEllipse(QPointF(lx, 18), 6, 6);
        p.setPen(QColor("#2b3a2f"));
        p.drawText(QRectF(lx + 10, 8, 80, 20), Qt::AlignVCenter | Qt::AlignLeft, item.first);
        lx += 92;
    }
}

// ---------------------------------------------------------------- helpers
void clearLayout(QLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *w = item->widget()) {
            w->hide();              // disparaît tout de suite, supprimé au prochain tour de boucle
            w->setParent(nullptr);
            w->deleteLater();
        }
        delete item;
    }
}

QTableWidget *makeTable(const QStringList &headers, const QList<QStringList> &rows, int statusCol)
{
    auto *t = new QTableWidget(rows.size(), headers.size());
    t->setHorizontalHeaderLabels(headers);
    t->verticalHeader()->setVisible(false);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setShowGrid(false);
    t->setAlternatingRowColors(true);
    t->setFocusPolicy(Qt::NoFocus);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->horizontalHeader()->setHighlightSections(false);
    for (int r = 0; r < rows.size(); ++r) {
        t->setRowHeight(r, 48);
        for (int c = 0; c < headers.size(); ++c) {
            if (c == statusCol) {
                auto *w = new QWidget;
                auto *l = new QHBoxLayout(w);
                l->setContentsMargins(8, 0, 8, 0);
                l->addWidget(Theme::badge(rows[r][c]), 0, Qt::AlignLeft | Qt::AlignVCenter);
                t->setCellWidget(r, c, w);
            } else {
                t->setItem(r, c, new QTableWidgetItem(rows[r][c]));
            }
        }
    }
    t->setMinimumHeight(48 * rows.size() + 44);
    return t;
}

QScrollArea *pageShell(const QString &title, const QString &subtitle, QVBoxLayout *&content)
{
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *inner = new QFrame;
    content = new QVBoxLayout(inner);
    content->setContentsMargins(28, 24, 28, 28);
    content->setSpacing(16);
    auto *t = new QLabel(title);
    t->setObjectName("pageTitle");
    content->addWidget(t);
    content->addWidget(Theme::muted(subtitle));
    scroll->setWidget(inner);
    return scroll;
}
