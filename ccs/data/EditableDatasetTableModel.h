#ifndef EditableDatasetTableModel_H
#define EditableDatasetTableModel_H

#include "../data/ChaosAbstractTableModel.h"

#include <chaos_service_common/data/dataset/DatasetAttribute.h>

class EditableDatasetTableModel:
        public ChaosAbstractTableModel {
public:
    EditableDatasetTableModel(QObject *parent=0);
    void addNewDatasetAttribute();
    void addNewElemenToToDataset(const chaos::service_common::data::dataset::DatasetAttribute& new_dataset_attribute);
    void removeElementFromDataset(const QString& attribute_name,
                                  const chaos::DataType::DataSetAttributeIOAttribute direction);
    void setDatasetAttributeList(chaos::service_common::data::dataset::DatasetAttributeList *master_attribute_list);
protected:
    int getRowCount() const;
    int getColumnCount() const;
    QString getHeaderForColumn(int column) const;
    QVariant getCellData(int row, int column) const;
    bool setCellData(const QModelIndex &index, const QVariant &value);
    bool isCellEditable(const QModelIndex &index) const;
    QVariant getTooltipTextForData(int row, int column) const;
    QVariant getTextAlignForData(int row, int column) const;

    QString decodeTypeToString(chaos::DataType::DataType type) const;
    chaos::DataType::DataType decodeStringToType(const QString &type_string);
private:
    //! reference to attribute list
    chaos::service_common::data::dataset::DatasetAttributeList *attribute_list;
};

#endif // EditableDatasetTableModel_H
