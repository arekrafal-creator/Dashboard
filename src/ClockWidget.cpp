#include "ClockWidget.h"
#include <QTimer>
#include <QDateTime>

ClockWidget::ClockWidget(QWidget *parent)
    : QLabel(parent)
{
    setObjectName("clockLabel");
    setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ClockWidget::updateTime);
    m_timer->start(1000);

    updateTime();
}

void ClockWidget::updateTime()
{
    setText(QDateTime::currentDateTime().toString("dddd, d MMMM yyyy   HH:mm:ss"));
}
