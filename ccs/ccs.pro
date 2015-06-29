#-------------------------------------------------
#
# Project created by QtCreator 2015-03-31T11:15:31
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = ccs
TEMPLATE = app
RESOURCES = qdarkstyle/style.qrc \
    dark_orange/dark_orange_style.qrc \
    image/image.qrc

INCLUDEPATH += $$PWD/../
INCLUDEPATH += $$PWD/../usr/local/include

LIBS += -L$$PWD/../usr/local/lib

LIBS +=     -lzmq\
            -lchaos_common\
            -lchaos_metadata_service_client\
            -lboost_chrono\
            -lboost_log\
            -lboost_log_setup\
            -lboost_system\
            -lboost_thread\
            -lboost_program_options\
            -lboost_regex\
            -lboost_filesystem

SOURCES += main.cpp\
    search/SearchNodeResult.cpp \
    search/SearchNode.cpp \
    presenter/PresenterWidget.cpp \
    presenter/CommandPresenter.cpp \
    api_async_processor/ApiAsyncProcessor.cpp \
    api_async_processor/ApiAsyncRunnable.cpp \
    MainController.cpp \
    node/unit_server/UnitServerEditor.cpp \
    node/control_unit/ControUnitInstanceEditor.cpp \
    node/control_unit/DriverDescriptionInputDialog.cpp \
    node/control_unit/AttributeValueRangeDialog.cpp \
    node/data_service/DataServiceEditor.cpp \
    node/data_service/CreateEditDataService.cpp \
    preference/PreferenceDialog.cpp \
    node/data/FixedOutputChannelDatasetTableModel.cpp \
    MainWindow.cpp \
    monitor/healt/HealtMonitorWidget.cpp \
    monitor/healt/HealtPresenterWidget.cpp \
    monitor/healt/HealtWidgetsListPresenteWidget.cpp \
    widget/LedIndicatorWidget.cpp \
    node/data/ChaosAbstractTableModel.cpp \
    node/control_unit/ControlUnitEditor.cpp \
    node/data/FixedInputChannelDatasetTableModel.cpp \
    widget/LedIndicatorAliveTSWidget.cpp \
    logic/property_switch/MultiPropertyLogicSwitch.cpp \
    logic/property_switch/SwitchAggregator.cpp \
    node/data/ChaosAbstractDataSetTableModel.cpp \
    node/data/ChaosAbstractListModel.cpp \
    node/data/CommandListModel.cpp \
    widget/list/delegate/TwoLineInformationListItemDelegate.cpp \
    widget/list/delegate/TwoLineInformationItem.cpp \
    node/data/CommandParameterTableModel.cpp \
    node/data/AttributeValueChangeSet.cpp \
    node/data/CommandTemplateListModel.cpp \
    node/control_unit/ControlUnitCommandTemplateEditor.cpp \
    node/control_unit/CommandTemplateInstanceEditor.cpp \
    data/CommandReader.cpp \
    data/CommandParameterReader.cpp \
    widget/CDSAttrQLineEdit.cpp \
    data/ChaosTypedAttributeValueSetter.cpp \
    external_lib/qcustomplot.cpp \
    data/AttributeReader.cpp

HEADERS  += \
    search/SearchNodeResult.h \
    search/SearchNode.h \
    presenter/PresenterWidget.h \
    presenter/CommandPresenter.h \
    api_async_processor/ApiAsyncProcessor.h \
    api_async_processor/ApiAsyncRunnable.h \
    MainController.h \
    node/unit_server/UnitServerEditor.h \
    node/control_unit/ControUnitInstanceEditor.h \
    node/control_unit/DriverDescriptionInputDialog.h \
    node/control_unit/AttributeValueRangeDialog.h \
    node/data_service/DataServiceEditor.h \
    node/data_service/CreateEditDataService.h \
    preference/PreferenceDialog.h \
    node/data/FixedOutputChannelDatasetTableModel.h \
    MainWindow.h \
    monitor/healt/HealtMonitorWidget.h \
    monitor/healt/HealtPresenterWidget.h \
    monitor/healt/HealtWidgetsListPresenteWidget.h \
    widget/LedIndicatorWidget.h \
    node/data/ChaosAbstractTableModel.h \
    node/control_unit/ControlUnitEditor.h \
    monitor/healt/handler/HealtStatusHandler.h \
    monitor/healt/handler/HealtHeartbeatHandler.h \
    monitor/healt/handler/handler.h \
    node/data/FixedInputChannelDatasetTableModel.h \
    widget/LedIndicatorAliveTSWidget.h \
    logic/property_switch/MultiPropertyLogicSwitch.h \
    logic/property_switch/SwitchAggregator.h \
    node/data/ChaosAbstractDataSetTableModel.h \
    monitor/healt/handler/AbstractAttributeHandler.h \
    monitor/healt/handler/MonitorBoolAttributeHandler.h \
    monitor/healt/handler/MonitorInt32AttributeHandler.h \
    monitor/healt/handler/MonitorInt64AttributeHandler.h \
    monitor/healt/handler/MonitorDoubleAttributeHandler.h \
    monitor/healt/handler/MonitorStringAttributeHandler.h \
    monitor/healt/handler/MonitorBinaryAttributeHandler.h \
    node/data/ChaosAbstractListModel.h \
    node/data/CommandListModel.h \
    widget/list/delegate/TwoLineInformationItem.h \
    widget/list/delegate/TwoLineInformationListItemDelegate.h \
    node/data/CommandParameterTableModel.h \
    node/data/AttributeValueChangeSet.h \
    node/data/CommandTemplateListModel.h \
    node/control_unit/ControlUnitCommandTemplateEditor.h \
    node/control_unit/CommandTemplateInstanceEditor.h \
    data/CommandReader.h \
    data/CommandParameterReader.h \
    widget/CDSAttrQLineEdit.h \
    data/ChaosTypedAttributeValueSetter.h \
    external_lib/qcustomplot.h \
    data/AttributeReader.h \
    monitor/system/handler/SystemControlUnit.h \
    monitor/handler/healt/HealtHeartbeatHandler.h \
    monitor/handler/healt/HealtStatusHandler.h \
    monitor/handler/AbstractAttributeHandler.h \
    monitor/handler/MonitorInt32AttributeHandler.h \
    monitor/handler/MonitorBinaryAttributeHandler.h \
    monitor/handler/MonitorBoolAttributeHandler.h \
    monitor/handler/MonitorDoubleAttributeHandler.h \
    monitor/handler/MonitorInt64AttributeHandler.h \
    monitor/handler/MonitorStringAttributeHandler.h \
    monitor/handler/handler.h \
    monitor/healt/healt.h \
    monitor/handler/healt/healt.h \
    monitor/handler/system/SystemControlUnitRunScheduleDelay.h \
    monitor/monitor.h \
    monitor/handler/system/system.h

FORMS    += \
    search/searchnoderesult.ui \
    search/searchnode.ui \
    node/unit_server/UnitServerEditor.ui \
    node/control_unit/ControUnitInstanceEditor.ui \
    node/control_unit/DriverDescriptionInputDialog.ui \
    node/control_unit/AttributeValueRangeDialog.ui \
    node/data_service/DataServiceEditor.ui \
    node/data_service/CreateEditDataService.ui \
    preference/PreferenceDialog.ui \
    MainWindow.ui \
    monitor/healt/HealtPresenterWidget.ui \
    monitor/healt/HealtWidgetsListPresenteWidget.ui \
    node/control_unit/ControlUnitEditor.ui \
    node/control_unit/ControlUnitCommandTemplateEditor.ui \
    node/control_unit/CommandTemplateInstanceEditor.ui

DISTFILES += \
    dark_orange.stylesheet
