#pragma once

#include "BankService.h"

#include <QDialog>

class QLineEdit;

class LoginWindow : public QDialog
{
public:
    explicit LoginWindow(BankService *service, QWidget *parent = nullptr);
    UserRecord authenticatedUser() const;

private:
    BankService *m_service;
    UserRecord m_authenticatedUser;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
};
