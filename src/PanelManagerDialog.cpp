#include "PanelManagerDialog.h"
#include "PanelEditDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSet>

PanelManagerDialog::PanelManagerDialog(const QVector<PanelConfig> &initialPanels, QWidget *parent)
    : QDialog(parent)
    , m_panels(initialPanels)
{
    setWindowTitle("Zarzadzaj kwerendami / panelami");
    setMinimumSize(480, 420);
    buildUi();
}

void PanelManagerDialog::buildUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *infoLabel = new QLabel(
        "Kazda pozycja na liscie to jeden kafelek na dashboardzie.\n"
        "Kolejnosc na liscie decyduje o ukladzie (od lewej do prawej, wiersz po wierszu).",
        this);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    auto *contentLayout = new QHBoxLayout();

    m_listWidget = new QListWidget(this);
    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &PanelManagerDialog::onItemDoubleClicked);
    contentLayout->addWidget(m_listWidget, 1);

    auto *buttonsLayout = new QVBoxLayout();
    auto *addButton = new QPushButton("Dodaj panel...", this);
    auto *editButton = new QPushButton("Edytuj...", this);
    auto *deleteButton = new QPushButton("Usun", this);
    auto *upButton = new QPushButton("Przesun w gore", this);
    auto *downButton = new QPushButton("Przesun w dol", this);

    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(editButton);
    buttonsLayout->addWidget(deleteButton);
    buttonsLayout->addSpacing(16);
    buttonsLayout->addWidget(upButton);
    buttonsLayout->addWidget(downButton);
    buttonsLayout->addStretch(1);

    contentLayout->addLayout(buttonsLayout);
    mainLayout->addLayout(contentLayout, 1);

    connect(addButton, &QPushButton::clicked, this, &PanelManagerDialog::onAddClicked);
    connect(editButton, &QPushButton::clicked, this, &PanelManagerDialog::onEditClicked);
    connect(deleteButton, &QPushButton::clicked, this, &PanelManagerDialog::onDeleteClicked);
    connect(upButton, &QPushButton::clicked, this, &PanelManagerDialog::onMoveUpClicked);
    connect(downButton, &QPushButton::clicked, this, &PanelManagerDialog::onMoveDownClicked);

    auto *closeLayout = new QHBoxLayout();
    closeLayout->addStretch(1);
    auto *closeButton = new QPushButton("Zamknij i zastosuj", this);
    closeButton->setDefault(true);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    closeLayout->addWidget(closeButton);
    mainLayout->addLayout(closeLayout);

    refreshListWidget();
}

void PanelManagerDialog::refreshListWidget()
{
    m_listWidget->clear();
    for (const PanelConfig &cfg : m_panels) {
        m_listWidget->addItem(cfg.title);
    }
}

int PanelManagerDialog::currentRow() const
{
    return m_listWidget->currentRow();
}

void PanelManagerDialog::onAddClicked()
{
    PanelConfig blank;
    PanelEditDialog dialog(blank, true, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    PanelConfig created = dialog.resultConfig();

    // Wygeneruj unikalne id na podstawie tytulu.
    QSet<QString> existingIds;
    for (const PanelConfig &p : m_panels)
        existingIds.insert(p.id);

    QString baseId = ConfigManager::slugify(created.title);
    QString candidateId = baseId;
    int suffix = 2;
    while (existingIds.contains(candidateId)) {
        candidateId = baseId + "_" + QString::number(suffix++);
    }
    created.id = candidateId;

    m_panels.push_back(created);
    refreshListWidget();
    m_listWidget->setCurrentRow(m_panels.size() - 1);
}

void PanelManagerDialog::onEditClicked()
{
    int row = currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Brak wyboru", "Zaznacz panel na liscie, ktory chcesz edytowac.");
        return;
    }

    PanelEditDialog dialog(m_panels[row], false, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    m_panels[row] = dialog.resultConfig();
    refreshListWidget();
    m_listWidget->setCurrentRow(row);
}

void PanelManagerDialog::onItemDoubleClicked()
{
    onEditClicked();
}

void PanelManagerDialog::onDeleteClicked()
{
    int row = currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Brak wyboru", "Zaznacz panel na liscie, ktory chcesz usunac.");
        return;
    }

    auto reply = QMessageBox::question(this, "Usun panel",
        QString("Czy na pewno usunac panel \"%1\"?").arg(m_panels[row].title));
    if (reply != QMessageBox::Yes)
        return;

    m_panels.remove(row);
    refreshListWidget();
}

void PanelManagerDialog::onMoveUpClicked()
{
    int row = currentRow();
    if (row <= 0)
        return;
    m_panels.swapItemsAt(row, row - 1);
    refreshListWidget();
    m_listWidget->setCurrentRow(row - 1);
}

void PanelManagerDialog::onMoveDownClicked()
{
    int row = currentRow();
    if (row < 0 || row >= m_panels.size() - 1)
        return;
    m_panels.swapItemsAt(row, row + 1);
    refreshListWidget();
    m_listWidget->setCurrentRow(row + 1);
}
