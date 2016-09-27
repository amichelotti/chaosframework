#ifndef CHAOSSTORAGETYPEWIDGET_H
#define CHAOSSTORAGETYPEWIDGET_H

#include <QWidget>
#include "ChaosMonitorWidgetCompanion.h"

namespace Ui {
class ChaosStorageTypeWidget;
}

class ChaosStorageTypeWidget :
        public QWidget,
        public ChaosMonitorWidgetCompanion,
        public chaos::metadata_service_client::node_monitor::ControlUnitMonitorHandler {
    Q_OBJECT
public:
    explicit ChaosStorageTypeWidget(QWidget *parent = 0);
    ~ChaosStorageTypeWidget();

    void initChaosContent();
    void deinitChaosContent();
    void updateChaosContent();

protected:
    void nodeChangedOnlineState(const std::string& node_uid,
                                chaos::metadata_service_client::node_monitor::OnlineState old_status,
                                chaos::metadata_service_client::node_monitor::OnlineState new_status);
    void updatedDS(const std::string& control_unit_uid,
                   int dataset_type,
                   chaos::metadata_service_client::node_monitor::MapDatasetKeyValues& dataset_key_values);

    void noDSDataFound(const std::string& control_unit_uid,
                       int dataset_type);
private slots:
    void updateUIStatus();

    void on_pushButtonLive_clicked();
    void on_pushButtonHistory_clicked();
    void on_pushButtonLiveAndHistory_clicked();
    void on_pushButtonUndefined_clicked();

private:
    bool data_found;
    chaos::DataServiceNodeDefinitionType::DSStorageType storage_type;
    chaos::metadata_service_client::node_monitor::OnlineState online_status;
    Ui::ChaosStorageTypeWidget *ui;

    void sendStorageType(chaos::DataServiceNodeDefinitionType::DSStorageType type);
    void apiHasStarted(const QString& api_tag);
    void apiHasEnded(const QString& api_tag);
};

#endif // CHAOSSTORAGETYPEWIDGET_H