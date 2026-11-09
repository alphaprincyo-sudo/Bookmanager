#pragma once
#include <QDialog>
#include "data.h"

class QLineEdit;
class QSpinBox;
class QVBoxLayout;
class QScrollArea;
class QWidget;

// Ajout d'un livre (avec saisie des codes-barres des exemplaires) ou modification (titre/auteur/ISBN).
class AddBookDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddBookDialog(QWidget *parent = nullptr);                        // ajout
    explicit AddBookDialog(const Book &existing, QWidget *parent = nullptr);  // modification
    int bookId() const { return m_bookId; }
    void accept() override;

private:
    void init(const QString &title);
    void rebuildCodes();

    bool m_edit = false;
    int m_bookId = 0;
    QLineEdit *m_title = nullptr, *m_author = nullptr, *m_isbn = nullptr;
    QSpinBox *m_count = nullptr;
    QWidget *m_codesBox = nullptr;
    QVBoxLayout *m_codesLayout = nullptr;
    QList<QLineEdit *> m_codes;
};

// Fiche d'un livre (double-clic) : informations, exemplaires et actions, liste d'attente.
class BookDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BookDialog(int bookId, QWidget *parent = nullptr);
    bool changed() const { return m_changed; }

private:
    void reload();
    QWidget *build();
    bool pickReader(const QString &title, const QString &label, int *readerId);
    void showError(const Outcome &o);
    void editBook();
    void removeBook();
    void addCopy();
    void lendCopy(int copyId);
    void returnCopy(int copyId);
    void removeCopy(int copyId);
    void joinWaitlist();
    void removeWait(int waitId);

    int m_id;
    bool m_changed = false;
    QScrollArea *m_scroll = nullptr;
};
