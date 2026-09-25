#include "QueryPanel.h"
#include "DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QSqlQueryModel>
#include <QTimer>
#include <QHeaderView>
#include <QDateTime>
#include <QColor>
#include <QBrush>

QueryPanel::QueryPanel(const PanelConfig &config, DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent)
    , m_config(config)
    , m_db(dbManager)
{
    buildUi();
}

void QueryPanel::buildUi()
{
    setObjectName("panelFrame");

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(14, 10, 14, 14);
    outerLayout->setSpacing(6);

    // --- Naglowek panelu: tytul po lewej, status odswiezenia po prawej ---
    auto *headerLayout = new QHBoxLayout();
    m_titleLabel = new QLabel(m_config.title, this);
    m_titleLabel->setObjectName("panelTitle");

    m_statusLabel = new QLabel("Oczekiwanie na pierwsze pobranie danych...", this);
    m_statusLabel->setObjectName("panelStatus");
    m_statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    headerLayout->addWidget(m_titleLabel, 1);
    headerLayout->addWidget(m_statusLabel, 0);
    outerLayout->addLayout(headerLayout);

    // --- Tabela z wynikami ---
    m_tableView = new QTableView(this);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setSelectionMode(QAbstractItemView::NoSelection);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    outerLayout->addWidget(m_tableView, 1);
}

void QueryPanel::startAutoRefresh(int defaultIntervalSec)
{
    int intervalSec = (m_config.refreshIntervalSec > 0) ? m_config.refreshIntervalSec : defaultIntervalSec;
    if (intervalSec < 5) intervalSec = 5; // zabezpieczenie przed zbyt czestym odpytywaniem bazy

    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &QueryPanel::refreshNow);
    }
    m_timer->start(intervalSec * 1000);

    // Pierwsze pobranie danych od razu, bez czekania na uplyw interwalu.
    refreshNow();
}

void QueryPanel::refreshNow()
{
    QString error;
    QSqlQueryModel *model = m_db->executeQuery(m_config.sql, error);
    model->setParent(this); // zeby Qt sam posprzatal pamiec

    // Podmieniamy model - stary (jesli byl) usuwamy recznie.
    QAbstractItemModel *oldModel = m_tableView->model();
    m_tableView->setModel(model);
    if (oldModel)
        oldModel->deleteLater();

    if (!error.isEmpty()) {
        m_statusLabel->setObjectName("panelStatusError");
        m_statusLabel->setText(error);
    } else {
        m_statusLabel->setObjectName("panelStatus");
        m_statusLabel->setText("Zaktualizowano: " + QDateTime::currentDateTime().toString("HH:mm:ss"));
        applyRowColoring();
    }
    // Wymuszenie ponownego zaladowania stylu po zmianie objectName (bledy na czerwono).
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
}

void QueryPanel::applyRowColoring()
{
    // Proste, opcjonalne kolorowanie wierszy w zaleznosci od panelu.
    // To jest jedyne miejsce z "logika biznesowa" - jesli chcesz inaczej
    // kolorowac swoje dane, dopisz tu kolejny "if" po wzorze ponizej.
    auto *model = qobject_cast<QSqlQueryModel *>(m_tableView->model());
    if (!model)
        return;

    // Kolorowanie dziala tylko wizualnie na poziomie danych tekstowych zwracanych
    // z bazy (nie zmienia samych danych), wiec jest bezpieczne do dowolnej edycji.
    if (m_config.id == "pick_status") {
        // Zaklada, ze pierwsza kolumna to nazwa statusu (RELEASED / HOLD)
        for (int row = 0; row < model->rowCount(); ++row) {
            QString statusText = model->data(model->index(row, 0)).toString().toUpper();
            QColor color = statusText.contains("HOLD") ? QColor("#ff5c5c") : QColor("#4be07a");
            for (int col = 0; col < model->columnCount(); ++col) {
                model->setData(model->index(row, col), QBrush(color), Qt::ForegroundRole);
            }
        }
    } else if (m_config.id == "top_bottom_pickers") {
        // Zaklada, ze ostatnia kolumna zawiera "TOP 3" albo "DOL 3"
        int lastCol = model->columnCount() - 1;
        for (int row = 0; row < model->rowCount(); ++row) {
            QString groupText = model->data(model->index(row, lastCol)).toString().toUpper();
            QColor color = groupText.contains("TOP") ? QColor("#4be07a") : QColor("#ff5c5c");
            for (int col = 0; col < model->columnCount(); ++col) {
                model->setData(model->index(row, col), QBrush(color), Qt::ForegroundRole);
            }
        }
    }
}
