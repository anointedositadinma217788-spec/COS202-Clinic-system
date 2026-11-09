#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include "model/ClinicDatabase.h"

namespace clinic_mgmt {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRegisterPatient();
    void onScheduleAppointment();
    void onManageAppointments();
    void onSearchSort();
    void onWaitingQueue();
    void onReports();

private:
    QTextEdit *outputArea;
    ClinicDatabase clinicDb;
};

}

#endif
