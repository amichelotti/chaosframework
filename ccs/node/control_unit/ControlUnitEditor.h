#ifndef CONTROLUNITEDITOR_H
#define CONTROLUNITEDITOR_H
#include "ControlUnitCommandTemplateEditor.h"

#include "../data/CommandListModel.h"
#include "../data/CommandTemplateListModel.h"
#include "../data/FixedInputChannelDatasetTableModel.h"
#include "../data/FixedOutputChannelDatasetTableModel.h"

#include "../../presenter/PresenterWidget.h"
#include "../../monitor/healt/handler/handler.h"
#include "../../logic/property_switch/SwitchAggregator.h"

#include <QWidget>
#include <QItemSelection>

namespace Ui {
class ControlUnitEditor;
}

class ControlUnitEditor :
        public PresenterWidget {
    Q_OBJECT

public:
    explicit ControlUnitEditor(const QString& _control_unit_unique_id);
    ~ControlUnitEditor();
protected:
    void initUI();
    bool canClose();
    void onApiDone(const QString& tag,
                   QSharedPointer<chaos::common::data::CDataWrapper> api_result);

private slots:
    void updateAttributeValue(const QString& key,
                              const QString& attribute_name,
                              const QVariant& attribute_value);

    void onLogicSwitchChangeState(const QString& switch_name,
                                  bool switch_activate);

    void handleSelectionChangedOnListWiew(const QItemSelection& selection,
                                          const QItemSelection& previous_selected);

    void templateSaved(const QString& tempalte_name,
                       const QString& command_uid);

    void on_pushButtonLoadAction_clicked();

    void on_pushButtonUnload_clicked();

    void on_pushButtonInitAction_clicked();

    void on_pushButtonDeinitAction_clicked();

    void on_pushButtonStartAction_clicked();

    void on_pushButtonStopAction_clicked();

    void on_checkBoxMonitorOutputChannels_clicked();

    void on_checkBoxMonitorInputChannels_clicked();

    void on_pushButtonCommitSet_clicked();

    void on_pushButtonResetChangeSet_clicked();

    void on_pushButtonAddNewCommadInstance_clicked();

    void on_pushButtonUpdateTemaplteList_clicked();

    void on_pushButtonEditInstance_clicked();

private:
    QString getStatusString(int status);

    void updateTemplateSearch();
    void updateAllControlUnitInfomration();

    void fillInfo(const QSharedPointer<chaos::common::data::CDataWrapper>& node_info);
    void fillDataset(const QSharedPointer<chaos::common::data::CDataWrapper>& dataset);

    //keep track of the last relevated online state
    bool last_online_state;
    const QString           control_unit_unique_id;
    QString                 unit_server_parent_unique_id;
    SwitchAggregator        logic_switch_aggregator;
    HealthHartbeatHandler   hb_handler;
    HealtStatusHandler                  status_handler;

    CommandListModel                    command_list_model;
    CommandTemplateListModel            command_template_list_model;

    FixedOutputChannelDatasetTableModel dataset_output_table_model;
    FixedInputChannelDatasetTableModel  dataset_input_table_model;
    Ui::ControlUnitEditor *ui;
};

#endif // CONTROLUNITEDITOR_H
