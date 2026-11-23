#pragma once

#include "BankService.h"

#include <QMainWindow>

class QStackedWidget;
class QPushButton;
class DashboardPage;
class ClientsPage;
class TransactionsPage;
class UsersPage;

class MainWindow : public QMainWindow
{
public:
    MainWindow(BankService *service, const UserRecord &user, QWidget *parent = nullptr);

private:
    BankService *m_service;
    UserRecord m_user;
    QStackedWidget *m_pages;
    DashboardPage *m_dashboardPage;
    ClientsPage *m_clientsPage;
    TransactionsPage *m_transactionsPage;
    UsersPage *m_usersPage;
    QPushButton *m_dashboardNav;
    QPushButton *m_clientsNav;
    QPushButton *m_transactionsNav;
    QPushButton *m_usersNav;

    void showPage(int index, QPushButton *activeButton);
    void logout();
};
