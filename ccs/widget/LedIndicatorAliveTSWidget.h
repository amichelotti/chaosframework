#ifndef LEDINDICATORALIVETSWIDGET_H
#define LEDINDICATORALIVETSWIDGET_H

#include "LedIndicatorAliveTSWidget.h"
#include "LedIndicatorWidget.h"
#include <QIcon>

class LedIndicatorAliveTSWidget:
        public LedIndicatorWidget {
public:
    LedIndicatorAliveTSWidget(QWidget *parent);
    void setNewTS(uint64_t current_timestamp);
private:
    QSharedPointer<QIcon> no_ts;
    QSharedPointer<QIcon> timeouted;
    QSharedPointer<QIcon> alive;
    uint64_t last_recevied_ts;
};

#endif // LEDINDICATORALIVETSWIDGET_H
