#pragma once
// =============================================================================
// ConnectionSettingsDialog
//
// Formularz do wpisania danych logowania do bazy danych - bezposrednio
// w aplikacji. Pokazuje sie automatycznie przy pierwszym uruchomieniu
// (gdy nie ma jeszcze zapisanej konfiguracji) oraz na zadanie z menu
// "Polaczenie -> Ustawienia polaczenia...".
//
// Ma przycisk "Testuj polaczenie", ktory probuje sie polaczyc od razu,
// zanim uzytkownik zatwierdzi/zapisze zmiany.
// =============================================================================

#include <QDialog>
#include "ConfigManager.h"

class QComboBox;
class QLineEdit;
class QSpinBox;
class QLabel;
class QStackedWidget;

class ConnectionSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConnectionSettingsDialog(const DbConfig &current, QWidget *parent = nullptr);

    // Wywolaj po QDialog::exec() == QDialog::Accepted
    DbConfig resultConfig() const;

private slots:
    void onDriverChanged(int index);
    void onTestConnectionClicked();
    void onSaveClicked();

private:
    void buildUi();
    DbConfig collectFormValues() const;

    DbConfig m_initial;

    QComboBox *m_driverCombo = nullptr;
    QStackedWidget *m_stack = nullptr;

    // Strona QODBC
    QLineEdit *m_connectionStringEdit = nullptr;

    // Strona QMYSQL / QPSQL
    QLineEdit *m_hostEdit = nullptr;
    QSpinBox  *m_portSpin = nullptr;
    QLineEdit *m_databaseEdit = nullptr;
    QLineEdit *m_userEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;

    QSpinBox *m_refreshIntervalSpin = nullptr;
    QLabel *m_testResultLabel = nullptr;
};
