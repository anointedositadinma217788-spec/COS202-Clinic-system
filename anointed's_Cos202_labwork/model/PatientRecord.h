#ifndef PATIENTRECORD_H
#define PATIENTRECORD_H

#include "Appointable.h"
#include <string>
#include <vector>

namespace model {

class Appointment;

// abstract base class for every type of patient in the clinic
class PatientRecord : public Appointable {
protected:
    std::string patientId;
    std::string name;
    int age;
    std::string contact;
    std::vector<Appointment*> appointments;

public:
    PatientRecord(const std::string& patientId, const std::string& name, int age,
                  const std::string& contact);
    virtual ~PatientRecord();

    std::string getId() const;
    std::string getName() const;
    int getAge() const;
    std::string getContact() const;

    void setName(const std::string& newName);
    void setAge(int newAge);
    void setContact(const std::string& newContact);

    virtual std::string getCategory() const = 0;
    virtual std::string getPriorityLabel() const = 0;
    virtual int getConsultationFee() const = 0;

    virtual std::string getDetails() const;
    virtual std::string getExtraInfo() const;
    virtual std::string getExtra1() const;
    virtual std::string getExtra2() const;
    virtual void setExtraFields(const std::string& first, const std::string& second);

    void scheduleAppointment(Appointment* appt);
    bool cancelAppointment(int apptNo);
    std::vector<Appointment*> getAppointments() const;

    void unlinkAppointment(int apptNo);
    int countAppointments(const std::string& status) const;
    std::string getNextAppointment() const;
};

}

#endif
