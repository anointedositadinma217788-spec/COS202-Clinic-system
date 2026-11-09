#include "mainwindow.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>

namespace clinic_mgmt {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    outputArea(nullptr)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QLabel *title = new QLabel("Clinic System - Patient & Appointment Manager", this);
    title->setStyleSheet("font-size: 15px; font-weight: bold; color: #1a3c5e;");
    mainLayout->addWidget(title);

    QGridLayout *buttonGrid = new QGridLayout();

    QPushButton *btnRegister = new QPushButton("Register Patient", this);
    QPushButton *btnSchedule = new QPushButton("Schedule Appointment", this);
    QPushButton *btnManage = new QPushButton("Manage Appointments", this);
    QPushButton *btnSearch = new QPushButton("Search & Sort Patients", this);
    QPushButton *btnQueue = new QPushButton("Waiting Queue", this);
    QPushButton *btnReports = new QPushButton("Reports", this);

    QString btnStyle = "QPushButton { background-color: #2e6da4; color: white; "
                       "border: none; padding: 8px; border-radius: 4px; } "
                       "QPushButton:hover { background-color: #1c4f7a; }";
    btnRegister->setStyleSheet(btnStyle);
    btnSchedule->setStyleSheet(btnStyle);
    btnManage->setStyleSheet(btnStyle);
    btnSearch->setStyleSheet(btnStyle);
    btnQueue->setStyleSheet(btnStyle);
    btnReports->setStyleSheet(btnStyle);

    buttonGrid->addWidget(btnRegister, 0, 0);
    buttonGrid->addWidget(btnSchedule, 0, 1);
    buttonGrid->addWidget(btnManage, 1, 0);
    buttonGrid->addWidget(btnSearch, 1, 1);
    buttonGrid->addWidget(btnQueue, 2, 0);
    buttonGrid->addWidget(btnReports, 2, 1);

    mainLayout->addLayout(buttonGrid);

    outputArea = new QTextEdit(this);
    outputArea->setReadOnly(true);
    outputArea->setStyleSheet("background-color: #f0f4f8; border: 1px solid #ccc;");
    mainLayout->addWidget(outputArea);

    central->setLayout(mainLayout);
    setCentralWidget(central);

    setWindowTitle("Clinic Management System");
    resize(550, 450);

    connect(btnRegister, SIGNAL(clicked()), this, SLOT(onRegisterPatient()));
    connect(btnSchedule, SIGNAL(clicked()), this, SLOT(onScheduleAppointment()));
    connect(btnManage, SIGNAL(clicked()), this, SLOT(onManageAppointments()));
    connect(btnSearch, SIGNAL(clicked()), this, SLOT(onSearchSort()));
    connect(btnQueue, SIGNAL(clicked()), this, SLOT(onWaitingQueue()));
    connect(btnReports, SIGNAL(clicked()), this, SLOT(onReports()));
}

MainWindow::~MainWindow() {}

void MainWindow::onRegisterPatient() {
    bool ok;
    QStringList categories;
    categories << "General" << "Emergency" << "FollowUp";
    QString category = QInputDialog::getItem(this, "Register Patient", "Patient Category:", categories, 0, false, &ok);
    if (!ok) return;

    QString reg = QInputDialog::getText(this, "Register Patient", "Reg Number:", QLineEdit::Normal, "", &ok);
    if (!ok || reg.isEmpty()) return;
    QString name = QInputDialog::getText(this, "Register Patient", "Full Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;
    QString ageStr = QInputDialog::getText(this, "Register Patient", "Age:", QLineEdit::Normal, "", &ok);
    if (!ok || ageStr.isEmpty()) return;
    QString phone = QInputDialog::getText(this, "Register Patient", "Phone:", QLineEdit::Normal, "", &ok);
    if (!ok || phone.isEmpty()) return;

    int age = ageStr.toInt();
    std::string sreg = reg.toStdString();
    std::string sname = name.toStdString();
    std::string sphone = phone.toStdString();

    PatientRecord* patient = nullptr;
    if (category == "General") {
        patient = new GeneralPatient(sreg, sname, age, sphone);
    } else if (category == "Emergency") {
        QString priorityStr = QInputDialog::getText(this, "Register Patient", "Priority (1-5):", QLineEdit::Normal, "1", &ok);
        if (!ok) return;
        QString complaint = QInputDialog::getText(this, "Register Patient", "Complaint:", QLineEdit::Normal, "", &ok);
        if (!ok) return;
        patient = new EmergencyPatient(sreg, sname, age, sphone, priorityStr.toInt(), complaint.toStdString());
    } else if (category == "FollowUp") {
        QString lastVisit = QInputDialog::getText(this, "Register Patient", "Last Visit Date:", QLineEdit::Normal, "", &ok);
        if (!ok) return;
        QString doctor = QInputDialog::getText(this, "Register Patient", "Doctor Name:", QLineEdit::Normal, "", &ok);
        if (!ok) return;
        patient = new FollowUpPatient(sreg, sname, age, sphone, lastVisit.toStdString(), doctor.toStdString());
    }

    if (patient) {
        clinicDb.addPatient(patient);
        outputArea->append("Registered: " + name + " (" + category + ")");
    }
}

void MainWindow::onScheduleAppointment() {
    bool ok;
    QString preg = QInputDialog::getText(this, "Schedule Appointment", "Patient Reg Number:", QLineEdit::Normal, "", &ok);
    if (!ok || preg.isEmpty()) return;

    PatientRecord* patient = clinicDb.findPatientByReg(preg.toStdString());
    if (!patient) {
        QMessageBox::warning(this, "Error", "Patient not found.");
        return;
    }

    QString doctor = QInputDialog::getText(this, "Schedule Appointment", "Doctor:", QLineEdit::Normal, "", &ok);
    if (!ok || doctor.isEmpty()) return;
    QString time = QInputDialog::getText(this, "Schedule Appointment", "Date/Time:", QLineEdit::Normal, "", &ok);
    if (!ok || time.isEmpty()) return;
    QString service = QInputDialog::getText(this, "Schedule Appointment", "Service:", QLineEdit::Normal, "", &ok);
    if (!ok || service.isEmpty()) return;

    Appointment* appt = new Appointment(clinicDb.getNextApptNum(), preg.toStdString(),
                                         doctor.toStdString(), time.toStdString(),
                                         service.toStdString(), "Scheduled");
    clinicDb.addAppointment(appt);
    outputArea->append("Appointment booked for " + preg + " with " + doctor);
}

void MainWindow::onManageAppointments() {
    bool ok;
    QString apptNumStr = QInputDialog::getText(this, "Manage Appointments", "Appointment Number to cancel:", QLineEdit::Normal, "", &ok);
    if (!ok || apptNumStr.isEmpty()) return;

    int apptNum = apptNumStr.toInt();
    if (clinicDb.cancelAppointment(apptNum)) {
        outputArea->append("Appointment " + apptNumStr + " cancelled.");
    } else {
        QMessageBox::warning(this, "Error", "Appointment not found.");
    }
}

void MainWindow::onSearchSort() {
    bool ok;
    QString reg = QInputDialog::getText(this, "Search Patient", "Reg Number to search:", QLineEdit::Normal, "", &ok);
    if (!ok || reg.isEmpty()) return;

    PatientRecord* patient = clinicDb.findPatientByReg(reg.toStdString());
    if (patient) {
        outputArea->append("Found: " + QString::fromStdString(patient->getFullName()) +
                           " - Category: " + QString::fromStdString(patient->getCategory()));
    } else {
        QMessageBox::information(this, "Search", "Patient not found.");
    }
}

void MainWindow::onWaitingQueue() {
    std::vector<PatientRecord*> all = clinicDb.getAllPatients();
    outputArea->append("=== Waiting Queue ===");
    for (size_t i = 0; i < all.size(); i++) {
        if (all[i]->getCategory() == "Emergency") {
            outputArea->append("[PRIORITY] " + QString::fromStdString(all[i]->getRegNumber()) +
                               " - " + QString::fromStdString(all[i]->getFullName()));
        }
    }
    for (size_t i = 0; i < all.size(); i++) {
        if (all[i]->getCategory() != "Emergency") {
            outputArea->append("[STANDARD] " + QString::fromStdString(all[i]->getRegNumber()) +
                               " - " + QString::fromStdString(all[i]->getFullName()));
        }
    }
}

void MainWindow::onReports() {
    std::vector<PatientRecord*> all = clinicDb.getAllPatients();
    int genCount = 0, emerCount = 0, followCount = 0;
    for (size_t i = 0; i < all.size(); i++) {
        std::string cat = all[i]->getCategory();
        if (cat == "General") genCount++;
        else if (cat == "Emergency") emerCount++;
        else if (cat == "FollowUp") followCount++;
    }

    std::vector<Appointment*> appts = clinicDb.getAllAppointments();
    int schedCount = 0, cancCount = 0, compCount = 0;
    for (size_t i = 0; i < appts.size(); i++) {
        std::string status = appts[i]->getApptStatus();
        if (status == "Scheduled") schedCount++;
        else if (status == "Cancelled") cancCount++;
        else if (status == "Completed") compCount++;
    }

    outputArea->append("=== Reports ===");
    outputArea->append("Total Patients: " + QString::number(all.size()));
    outputArea->append("  General: " + QString::number(genCount));
    outputArea->append("  Emergency: " + QString::number(emerCount));
    outputArea->append("  FollowUp: " + QString::number(followCount));
    outputArea->append("Total Appointments: " + QString::number(appts.size()));
    outputArea->append("  Scheduled: " + QString::number(schedCount));
    outputArea->append("  Cancelled: " + QString::number(cancCount));
    outputArea->append("  Completed: " + QString::number(compCount));
}

}
