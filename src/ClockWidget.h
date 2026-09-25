#pragma once
// =============================================================================
// ClockWidget
//
// Maly, niezalezny widget pokazujacy aktualna date i godzine.
// Przydatny na duzym ekranie (np. w hali), zeby od razu bylo widac,
// czy dane sa "swieze". Aktualizuje sie raz na sekunde.
// =============================================================================

#include <QLabel>

class QTimer;

class ClockWidget : public QLabel {
    Q_OBJECT
public:
    explicit ClockWidget(QWidget *parent = nullptr);

private slots:
    void updateTime();

private:
    QTimer *m_timer;
};
