#pragma once
// =============================================================================
// PanelEditDialog
//
// Formularz do dodania NOWEGO panelu albo edycji istniejacego.
// Tu wklejasz swoje zapytanie SQL - bezposrednio w programie, bez otwierania
// zadnego pliku tekstowego.
// =============================================================================

#include <QDialog>
#include "ConfigManager.h"

class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QCheckBox;

class PanelEditDialog : public QDialog {
    Q_OBJECT
public:
    // isNew = true -> okno "Dodaj nowy panel", isNew = false -> "Edytuj panel"
    explicit PanelEditDialog(const PanelConfig &initial, bool isNew, QWidget *parent = nullptr);

    PanelConfig resultConfig() const;

private slots:
    void onSaveClicked();

private:
    void buildUi(bool isNew);

    PanelConfig m_initial;
    bool m_isNew;

    QLineEdit *m_titleEdit = nullptr;
    QPlainTextEdit *m_sqlEdit = nullptr;
    QCheckBox *m_customRefreshCheck = nullptr;
    QSpinBox *m_refreshIntervalSpin = nullptr;
};
