#include "CNodeLogWidget.h"
#include "ui_CNodeLogWidget.h"

#include <QDebug>

using namespace chaos::metadata_service_client;
using namespace chaos::metadata_service_client::api_proxy;

CNodeLogWidget::CNodeLogWidget(QWidget *parent):
    QWidget(parent),
    ChaosWidgetCompanion(),
    ui(new Ui::CNodeLogWidget) {
    ui->setupUi(this);
    QList<int> sizes;
    sizes << (size().width()*1/5) << (size().width()*4/5);
    ui->splitterMainHorizontal->setSizes(sizes);
}

CNodeLogWidget::~CNodeLogWidget() {
    delete ui;
}

void CNodeLogWidget::initChaosContent() {
    //add the list model to log domain list
    ui->listViewLogTypes->setModel(&domain_list_model);
    //connect for signal when user check or uncheck the log domains
    connect(&domain_list_model,
            SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            SLOT(logTypesDataChanged(QModelIndex,QModelIndex,QVector<int>)));

    //configure the entry table
    ui->tableViewLogEntries->setModel(&entry_table_model);
    ui->tableViewLogEntries->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewLogEntries->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewLogEntries->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewLogEntries->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->tableViewLogEntries->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SLOT(logEntriesTableSelectionChanged(QModelIndex,QModelIndex)));

    //set table view for log data
    ui->tableViewLogData->setModel(&data_table_model);
    ui->tableViewLogData->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewLogData->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewLogData->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewLogData->setSelectionMode(QAbstractItemView::MultiSelection);

    setPageLength(4);

    //register for log
    ChaosMetadataServiceClient::getInstance()->registerEventHandler(this);
}

void CNodeLogWidget::deinitChaosContent() {
    //remove log handler
    ChaosMetadataServiceClient::getInstance()->deregisterEventHandler(this);
}

void CNodeLogWidget::updateChaosContent() {
    domain_list_model.updateDomainListForUID(nodeUID());
}

void CNodeLogWidget::setPageLength(qint32 page_length) {
    entry_table_model.setPageLength(p_page_length = page_length);
}

qint32 CNodeLogWidget::pageLength() {
    return p_page_length;
}

void CNodeLogWidget::handleLogEvent(const std::string source,
                                    const std::string domain) {
    //if widget i snot visible we do nothing
    if(isVisible()) return;

    if(source.compare(nodeUID().toStdString()) == 0) {
        //the event if for my node
        if(domain_list_model.isDomainChecked(QString::fromStdString(domain))) {
            qDebug()<< "Update event for log";
            //we need to update the log
            domain_list_model.updateDomainListForUID(nodeUID());
        }
    }
}

void CNodeLogWidget::logTypesDataChanged(const QModelIndex& top_left,
                                         const QModelIndex& bottom_right,
                                         const QVector<int>& roles) {
    //some domain has been checked or unchecked
    logging::LogDomainList domain_list;
    domain_list_model.getActiveDomains(domain_list);
    entry_table_model.updateEntries(nodeUID(),
                                    domain_list);
}

void CNodeLogWidget::logEntriesTableSelectionChanged(const QModelIndex& current_selection,
                                                     const QModelIndex& previous_selection) {
    if(current_selection.isValid()) {
        data_table_model.setLogEntry(entry_table_model.getLogEntryForRow(current_selection.row()));
    } else {
        data_table_model.clear();
    }


}

void CNodeLogWidget::on_pushButtonUpdateLogTypes_clicked() {
    domain_list_model.updateDomainListForUID(nodeUID());
}

void CNodeLogWidget::on_pushButtonLogEntriesNextPage_clicked() {
    entry_table_model.nextPage();
}

void CNodeLogWidget::on_pushButtonLogEntriesPreviousPage_clicked() {
    entry_table_model.previousPage();
}
