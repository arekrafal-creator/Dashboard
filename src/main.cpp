// =============================================================================
// main.cpp - punkt startowy aplikacji.
// Celowo bardzo krotki - cala logika jest w MainWindow i pozostalych klasach.
// =============================================================================

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Dashboard Magazynowy");

    MainWindow window;
    window.showMaximized(); // start w trybie zmaksymalizowanym - wygodne na duzym ekranie
    // Wskazowka: nacisnij F11 w aplikacji, aby przejsc w pelny ekran (bez paska tytulowego).

    return app.exec();
}
