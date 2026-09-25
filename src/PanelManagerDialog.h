#pragma once
// =============================================================================
// PanelManagerDialog
//
// Lista wszystkich paneli na dashboardzie z przyciskami:
// Dodaj / Edytuj / Usun / Przesun w gore / Przesun w dol.
// Kolejnosc na liscie = kolejnosc wyswietlania na dashboardzie
// (uklad siatki jest wyliczany automatycznie przez MainWindow).
// =============================================================================

#include <QDialog>
#include <QVector>
#include "ConfigManager.h"

class QListWidget;

class PanelManagerDialog : public QDialog {
    Q_OBJECT
public:
    explicit PanelManagerDialog(const QVector<PanelConfig> &initialPanels, QWidget *parent = nullptr);

    QVector<PanelConfig> resultPanels() const { return m_panels; }

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onItemDoubleClicked();

private:
    void buildUi();
    void refreshListWidget();
    int currentRow() const;

    QVector<PanelConfig> m_panels;
    QListWidget *m_listWidget = nullptr;
};
