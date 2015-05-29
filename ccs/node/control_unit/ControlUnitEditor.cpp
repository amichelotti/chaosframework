#include "ControlUnitEditor.h"
#include "ui_controluniteditor.h"

#include <QDateTime>
#include <QMessageBox>
#include <QDebug>
static const QString TAG_CU_INFO = QString("g_cu_i");
static const QString TAG_CU_DATASET = QString("g_cu_d");
static const QString TAG_CU_INSTANCE = QString("g_cu_instance");
static const QString TAG_CU_APPLY_CHANGESET = QString("g_cu_apply_changeset");

using namespace chaos::common::data;
using namespace chaos::metadata_service_client;
using namespace chaos::metadata_service_client::api_proxy;

ControlUnitEditor::ControlUnitEditor(const QString &_control_unit_unique_id) :
    PresenterWidget(NULL),
    last_online_state(false),
    control_unit_unique_id(_control_unit_unique_id),
    ui(new Ui::ControlUnitEditor),
    hb_handler(),
    status_handler(),
    dataset_output_table_model(control_unit_unique_id,
                               chaos::DataPackCommonKey::DPCK_DATASET_TYPE_OUTPUT),
    dataset_input_table_model(control_unit_unique_id,
                              chaos::DataPackCommonKey::DPCK_DATASET_TYPE_INPUT) {
    ui->setupUi(this);
    //handler connection
    connect(&status_handler,
            SIGNAL(valueUpdated(QString,QString,QVariant)),
            SLOT(updateAttributeValue(QString,QString,QVariant)));
    connect(&hb_handler,
            SIGNAL(valueUpdated(QString,QString,QVariant)),
            SLOT(updateAttributeValue(QString,QString,QVariant)));
}

ControlUnitEditor::~ControlUnitEditor() {
    //shutdown monitoring of channel
    dataset_input_table_model.setAttributeMonitoring(false);
    dataset_output_table_model.setAttributeMonitoring(false);
    //remove monitoring on cu and us
    if(unit_server_parent_unique_id.size()) {
        //remove old unit server for healt
        ChaosMetadataServiceClient::getInstance()->removeKeyAttributeHandlerForHealt(unit_server_parent_unique_id.toStdString(),
                                                                                     20,
                                                                                     &status_handler);
        ChaosMetadataServiceClient::getInstance()->removeKeyAttributeHandlerForHealt(unit_server_parent_unique_id.toStdString(),
                                                                                     20,
                                                                                     &hb_handler);
    }
    ChaosMetadataServiceClient::getInstance()->removeKeyAttributeHandlerForHealt(control_unit_unique_id.toStdString(),
                                                                                 20,
                                                                                 &status_handler);
    ChaosMetadataServiceClient::getInstance()->removeKeyAttributeHandlerForHealt(control_unit_unique_id.toStdString(),
                                                                                 20,
                                                                                 &hb_handler);
    delete ui;
}

void ControlUnitEditor::initUI() {
    //add model to table
    ui->tableViewOutputChannel->setModel(&dataset_output_table_model);
    ui->tableViewOutputChannel->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableViewInputChannels->setModel(&dataset_input_table_model);
    ui->tableViewInputChannels->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ChaosMetadataServiceClient::getInstance()->addKeyAttributeHandlerForHealt(control_unit_unique_id.toStdString(),
                                                                              20,
                                                                              &status_handler);
    ChaosMetadataServiceClient::getInstance()->addKeyAttributeHandlerForHealt(control_unit_unique_id.toStdString(),
                                                                              20,
                                                                              &hb_handler);

    //compose logic on switch
    //for load button
    logic_switch_aggregator.addNewLogicSwitch("cu_can_operate");
    logic_switch_aggregator.addKeyRefValue("cu_can_operate", "us_alive","true");
    logic_switch_aggregator.addKeyRefValue("cu_can_operate", "us_state","Load");

    logic_switch_aggregator.addNewLogicSwitch("load");
    logic_switch_aggregator.connectSwitch("load", "cu_can_operate");
    logic_switch_aggregator.addKeyRefValue("load", "cu_alive","false");
    logic_switch_aggregator.attachObjectAttributeToSwitch<bool>("load", ui->pushButtonLoadAction, "enabled", true, false);

    logic_switch_aggregator.addNewLogicSwitch("unload");
    logic_switch_aggregator.connectSwitch("unload", "cu_can_operate");
    logic_switch_aggregator.addKeyRefValue("unload", "cu_alive","true");
    logic_switch_aggregator.addKeyRefValue("unload", "cu_state","Load");
    logic_switch_aggregator.addKeyRefValue("unload", "cu_state","Deinit");
    logic_switch_aggregator.attachObjectAttributeToSwitch<bool>("unload", ui->pushButtonUnload, "enabled", true, false);
    logic_switch_aggregator.attachObjectAttributeToSwitch<bool>("unload", ui->pushButtonInitAction, "enabled", true, false);

    logic_switch_aggregator.addNewLogicSwitch("start");
    logic_switch_aggregator.connectSwitch("start", "cu_can_operate");
    logic_switch_aggregator.addKeyRefValue("start", "cu_alive","true");
    logic_switch_aggregator.addKeyRefValue("start", "cu_state","Init");
    logic_switch_aggregator.addKeyRefValue("start", "cu_state","Stop");
    logic_switch_aggregator.attachObjectAttributeToSwitch<bool>("start", ui->pushButtonStartAction, "enabled", true, false);
    logic_switch_aggregator.attachObjectAttributeToSwitch<bool>("start", ui->pushButtonDeinitAction, "enabled", true, false);

    logic_switch_aggregator.addNewLogicSwitch("stop");
    logic_switch_aggregator.connectSwitch("stop", "cu_can_operate");
    logic_switch_aggregator.addKeyRefValue("stop", "cu_alive","true");
    logic_switch_aggregator.addKeyRefValue("stop", "cu_state","Start");
    logic_switch_aggregator.attachObjectAttributeToSwitch<bool>("stop", ui->pushButtonStopAction, "enabled", true, false);

    //set control unit uid label
    ui->labelUniqueIdentifier->setText(control_unit_unique_id);


    //launch api for control unit information
    updateAllControlunitInfomration();
}


void ControlUnitEditor::updateAllControlunitInfomration() {
    submitApiResult(QString(TAG_CU_INFO),
                    GET_CHAOS_API_PTR(node::GetNodeDescription)->execute(control_unit_unique_id.toStdString()));
    submitApiResult(QString(TAG_CU_DATASET),
                    GET_CHAOS_API_PTR(control_unit::GetCurrentDataset)->execute(control_unit_unique_id.toStdString()));
    submitApiResult(QString(TAG_CU_INSTANCE),
                    GET_CHAOS_API_PTR(control_unit::GetInstance)->execute(control_unit_unique_id.toStdString()));
}


bool ControlUnitEditor::canClose() {
    return true;
}

void ControlUnitEditor::onApiDone(const QString& tag,
                                  QSharedPointer<CDataWrapper> api_result) {
    if(tag.compare(TAG_CU_INFO) == 0) {
        fillInfo(api_result);
    } else if(tag.compare(TAG_CU_DATASET) == 0) {
        fillDataset(api_result);
    } else if(tag.compare(TAG_CU_INSTANCE) == 0) {
        if(api_result.isNull()) return;
        if(api_result->hasKey(chaos::NodeDefinitionKey::NODE_PARENT)){
            const std::string new_u_s = api_result->getStringValue(chaos::NodeDefinitionKey::NODE_PARENT);
            if(unit_server_parent_unique_id.compare(QString::fromStdString(new_u_s)) != 0) {
                //whe ahve unit server changed
                if(unit_server_parent_unique_id.size()) {
                    //remove old unit server for healt
                    ChaosMetadataServiceClient::getInstance()->removeKeyAttributeHandlerForHealt(unit_server_parent_unique_id.toStdString(),
                                                                                                 20,
                                                                                                 &status_handler);
                    ChaosMetadataServiceClient::getInstance()->removeKeyAttributeHandlerForHealt(unit_server_parent_unique_id.toStdString(),
                                                                                                 20,
                                                                                                 &hb_handler);
                }

                ChaosMetadataServiceClient::getInstance()->addKeyAttributeHandlerForHealt(new_u_s,
                                                                                          20,
                                                                                          &status_handler);
                ChaosMetadataServiceClient::getInstance()->addKeyAttributeHandlerForHealt(new_u_s,
                                                                                          20,
                                                                                          &hb_handler);
                // keep track of new us uid
                unit_server_parent_unique_id = QString::fromStdString(new_u_s);
            }
            //apply control unit instance
            dataset_input_table_model.updateInstanceDescription(api_result);
        }
    } else if(tag.compare(TAG_CU_APPLY_CHANGESET) == 0) {
        dataset_input_table_model.applyChangeSet(true);
    }
}

void ControlUnitEditor::fillInfo(const QSharedPointer<chaos::common::data::CDataWrapper>& node_info) {
    if(node_info->hasKey(chaos::NodeDefinitionKey::NODE_RPC_ADDR)) {
        ui->labelRemoteAddress->setText(QString::fromStdString(node_info->getStringValue(chaos::NodeDefinitionKey::NODE_RPC_ADDR)));
    } else {
        ui->labelRemoteAddress->setText(tr("---------"));
    }

    if(node_info->hasKey(chaos::NodeDefinitionKey::NODE_RPC_DOMAIN)) {
        ui->labelRpcDomain->setText(QString::fromStdString(node_info->getStringValue(chaos::NodeDefinitionKey::NODE_RPC_DOMAIN)));
    } else {
        ui->labelRpcDomain->setText(tr("---------"));
    }
}

void ControlUnitEditor::updateAttributeValue(const QString& key,
                                             const QString& attribute_name,
                                             const QVariant& attribute_value) {
    if(key.startsWith(control_unit_unique_id)) {
        if(attribute_name.compare(chaos::NodeHealtDefinitionKey::NODE_HEALT_STATUS) == 0) {
            //print the status
            if(attribute_value.isNull()) {
                ui->labelControlUnitStatus->setText(tr("-------"));
            }else{
                ui->labelControlUnitStatus->setText(attribute_value.toString());
            }
            //broadcast cu status to switch
            logic_switch_aggregator.broadcastCurrentValueForKey("cu_state", attribute_value.toString());
            if( attribute_value.toString().compare(tr("Load"))==0){
                bool current_relevated_online_status = ui->ledIndicatorHealtTSControlUnit->getState()==2;
                if(current_relevated_online_status != last_online_state) {
                    if(current_relevated_online_status) {
                        //we need to reload all information
                        updateAllControlunitInfomration();
                    }
                    last_online_state = current_relevated_online_status;
                }
            }
        } else if(attribute_name.compare(chaos::NodeHealtDefinitionKey::NODE_HEALT_TIMESTAMP) == 0){
            //print the timestamp and update the red/green indicator
            ui->ledIndicatorHealtTSControlUnit->setNewTS(attribute_value.toULongLong());
            //broadcast cu status to switch
            logic_switch_aggregator.broadcastCurrentValueForKey("cu_alive", ((ui->ledIndicatorHealtTSControlUnit->getState()==2)?"true":"false"));
        }
    } else  if(key.startsWith(unit_server_parent_unique_id)) {
        //show healt for unit server
        if(attribute_name.compare(chaos::NodeHealtDefinitionKey::NODE_HEALT_STATUS) == 0) {
            //print the status
            if(attribute_value.isNull()) {
                ui->labelUnitServerStatus->setText(tr("-------"));
            }else{
                ui->labelUnitServerStatus->setText(attribute_value.toString());
            }
            logic_switch_aggregator.broadcastCurrentValueForKey("us_state", attribute_value.toString());
        } else if(attribute_name.compare(chaos::NodeHealtDefinitionKey::NODE_HEALT_TIMESTAMP) == 0){
            //print the timestamp and update the red/green indicator
            ui->ledIndicatorHealtTSUnitServer->setNewTS(attribute_value.toULongLong());
            logic_switch_aggregator.broadcastCurrentValueForKey("us_alive", (ui->ledIndicatorHealtTSUnitServer->getState()==2)?"true":"false");
        }
    }
}

void ControlUnitEditor::onLogicSwitchChangeState(const QString& switch_name,
                                                 bool switch_activate) {

}

void ControlUnitEditor::fillDataset(const QSharedPointer<chaos::common::data::CDataWrapper>& dataset) {
    dataset_output_table_model.updateData(dataset);
    dataset_input_table_model.updateData(dataset);
}

void ControlUnitEditor::on_pushButtonLoadAction_clicked() {
    submitApiResult("cu_laod",
                    GET_CHAOS_API_PTR(unit_server::LoadUnloadControlUnit)->execute(control_unit_unique_id.toStdString(), true));
}

void ControlUnitEditor::on_pushButtonUnload_clicked() {
    submitApiResult("cu_unlaod",
                    GET_CHAOS_API_PTR(unit_server::LoadUnloadControlUnit)->execute(control_unit_unique_id.toStdString(), false));
}

void ControlUnitEditor::on_pushButtonInitAction_clicked() {
    submitApiResult("cu_init",
                    GET_CHAOS_API_PTR(control_unit::InitDeinit)->execute(control_unit_unique_id.toStdString(), true));
}

void ControlUnitEditor::on_pushButtonDeinitAction_clicked() {
    submitApiResult("cu_deinit",
                    GET_CHAOS_API_PTR(control_unit::InitDeinit)->execute(control_unit_unique_id.toStdString(), false));
}

void ControlUnitEditor::on_pushButtonStartAction_clicked() {
    submitApiResult("cu_start",
                    GET_CHAOS_API_PTR(control_unit::StartStop)->execute(control_unit_unique_id.toStdString(), true));
}

void ControlUnitEditor::on_pushButtonStopAction_clicked() {
    submitApiResult("cu_stop",
                    GET_CHAOS_API_PTR(control_unit::StartStop)->execute(control_unit_unique_id.toStdString(), false));
}

void ControlUnitEditor::on_checkBoxMonitorOutputChannels_clicked() {
    dataset_output_table_model.setAttributeMonitoring(ui->checkBoxMonitorOutputChannels->isChecked());
}

void ControlUnitEditor::on_checkBoxMonitorInputChannels_clicked() {
    dataset_input_table_model.setAttributeMonitoring(ui->checkBoxMonitorInputChannels->isChecked());
}

void ControlUnitEditor::on_pushButtonCommitSet_clicked() {
    std::vector< boost::shared_ptr< control_unit::ControlUnitInputDatasetChangeSet > > value_set_array;
    dataset_input_table_model.getAttributeChangeSet(value_set_array);
    if(value_set_array[0]->change_set.size()==0) {
        QMessageBox msgBox;
        msgBox.setText(tr("Error applying changeset."));
        msgBox.setInformativeText(tr("No updated input attribute has been found!."));
        msgBox.exec();
        return;
    }
    submitApiResult("cu_apply_changeset",
                    GET_CHAOS_API_PTR(control_unit::SetInputDatasetAttributeValues)->execute(value_set_array));
}

void ControlUnitEditor::on_pushButtonResetChangeSet_clicked() {
    dataset_input_table_model.applyChangeSet(false);
}
