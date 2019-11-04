#include "ChaosBaseDatasetUI.h"
#include <QVariant>
#include <QMetaMethod>
#include <QMenu>
#include <QAction>
#include <QDebug>

ChaosBaseDatasetUI::ChaosBaseDatasetUI(QWidget *parent):
    QWidget(parent) {}

void ChaosBaseDatasetUI::setDeviceID(const QString &new_device_id) {
    p_device_id = new_device_id;
}

QString ChaosBaseDatasetUI::deviceID() const {
    return p_device_id;
}

void ChaosBaseDatasetUI::setDatasetType(const DatasetType &new_dataset_type) {
    p_dataset_type = new_dataset_type;
}

ChaosBaseDatasetUI::DatasetType ChaosBaseDatasetUI::datasetType() const {
    return p_dataset_type;
}

void ChaosBaseDatasetUI::updateData(QString attribute_name,
                                    QVariant attribute_value) {
    Q_UNUSED(attribute_name)
    QMetaObject::invokeMethod(this,
                              "updateValue",
                              Qt::QueuedConnection,
                              Q_ARG(QVariant, attribute_value));
    //emit the changed value
    emit attributeValueChanged(p_value, attribute_value);
    //keep track of the new value
    p_value = attribute_value;
}
