#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "presenter/CommandPresenter.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    CommandPresenter *command_presenter;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpenNode_triggered();

    void on_actionSearch_Node_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
