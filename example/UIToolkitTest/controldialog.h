/*
 * Copyright 2012, 2017 INFN
 *
 * Licensed under the EUPL, Version 1.2 or – as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */
#ifndef CONTROLDIALOG_H
#define CONTROLDIALOG_H

#include <QDialog>

#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#include <chaos/common/data/CUSchemaDB.h>
#endif
#include <string>

namespace Ui {
class ControlDialog;
}

namespace chaos {
   // class   CUSchemaDB;
   // struct  RangeValueInfo;

    namespace ui {
        class DeviceController;
    }
}


class ControlDialog : public QDialog
{
    Q_OBJECT
    chaos::ui::DeviceController *deviceController;
    chaos::common::data::RangeValueInfo attributerange;
    std::string attributeName;
    std::string deviceID;
    QWidget *controlWidget;
public:
    explicit ControlDialog(QWidget *parent = 0);
    ~ControlDialog();
    
    void initDialog( chaos::ui::DeviceController *_deviceController, std::string& _attributeName);
private slots:
    void on_buttonCommit_clicked();

    void on_buttonClose_clicked();

    void on_horizontalSlider_sliderMoved(int position);

private:
    Ui::ControlDialog *ui;
};

#endif // CONTROLDIALOG_H
