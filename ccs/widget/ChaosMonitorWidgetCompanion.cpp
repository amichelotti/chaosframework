#include "ChaosMonitorWidgetCompanion.h"

#include <QDebug>

using namespace chaos;
using namespace chaos::common::data;
using namespace chaos::metadata_service_client;
using namespace chaos::metadata_service_client::node_monitor;

ChaosMonitorWidgetCompanion::ChaosMonitorWidgetCompanion(chaos::metadata_service_client::node_monitor::ControllerType _monitor_controller_type,
                                                         node_monitor::NodeMonitorHandler *_monitor_handler):
    ChaosWidgetCompanion(),
    monitor_handler(_monitor_handler),
    monitor_controller_type(_monitor_controller_type),
    isTracking(false) {}

ChaosMonitorWidgetCompanion::~ChaosMonitorWidgetCompanion() {}

bool ChaosMonitorWidgetCompanion::trackNode() {
    return ChaosMetadataServiceClient::getInstance()->addHandlerToNodeMonitor(nodeUID().toStdString(),
                                                                              monitor_controller_type,
                                                                              monitor_handler);
}

bool ChaosMonitorWidgetCompanion::untrackNode() {
    return ChaosMetadataServiceClient::getInstance()->removeHandlerToNodeMonitor(nodeUID().toStdString(),
                                                                                 monitor_controller_type,
                                                                                 monitor_handler);
}

QString ChaosMonitorWidgetCompanion::datasetValueToLabel(const QString& attribute,
                                                        MapDatasetKeyValues& map_health_dataset,
                                                         unsigned int double_precision) {

    QString result = "-----";
    if(map_health_dataset.count(attribute.toStdString())) {
        CDataVariant custom_attribute_variant = map_health_dataset[attribute.toStdString()];
        switch(custom_attribute_variant.getType()) {
        case DataType::TYPE_BOOLEAN:
            result = custom_attribute_variant.asBool()?"True":"False";
            break;
        case DataType::TYPE_INT32:
            result = QString::number(custom_attribute_variant.asInt32());
            break;
        case DataType::TYPE_INT64:
            result = QString::number(custom_attribute_variant.asInt64());
            break;
        case DataType::TYPE_DOUBLE:
            result = QString::number(custom_attribute_variant.asDouble(), 'f', double_precision);
            break;
        case DataType::TYPE_STRING:
            result = QString::fromStdString(custom_attribute_variant.asString());
            break;
        case DataType::TYPE_BYTEARRAY:
            CDataBuffer data_buffer = custom_attribute_variant.asCDataBuffer();
            QByteArray byte_array =  QByteArray::fromRawData(data_buffer.getBuffer(),data_buffer.getBufferSize());
            result = byte_array.toBase64();
            break;
        }
    }
    return result;
}
