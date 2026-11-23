#pragma once

#include "BankService.h"

#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <functional>
#include <utility>

class QTableWidget;
class QLineEdit;
class QPushButton;

class DashboardPage : public QWidget
{
public:
    explicit DashboardPage(BankService *service, const UserRecord &user, QWidget *parent = nullptr);
    void refresh();

private:
    BankService *m_service;
    UserRecord m_user;
    QLabel *m_clientCountValue;
    QLabel *m_balanceValue;
    QLabel *m_accessValue;
};

class ClientsPage : public QWidget
{
public:
    explicit ClientsPage(BankService *service, const UserRecord &user, QWidget *parent = nullptr);
    void refresh();
    void setOnDataChanged(std::function<void()> callback) { m_onDataChanged = std::move(callback); }

private:
    BankService *m_service;
    UserRecord m_user;
    QTableWidget *m_table;
    QLineEdit *m_searchEdit;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_findButton;
    std::function<void()> m_onDataChanged;

    QString selectedAccountNumber() const;
    void populate(const QVector<ClientRecord> &clients);
    void addClient();
    void editClient();
    void deleteClient();
    void findClient();
};

class TransactionsPage : public QWidget
{
public:
    explicit TransactionsPage(BankService *service, const UserRecord &user, QWidget *parent = nullptr);
    void refreshBalances();
    void setOnDataChanged(std::function<void()> callback) { m_onDataChanged = std::move(callback); }

private:
    BankService *m_service;
    UserRecord m_user;
    QTableWidget *m_balancesTable;
    QLineEdit *m_depositAccount;
    QLineEdit *m_withdrawAccount;
    QDoubleSpinBox *m_depositAmount;
    QDoubleSpinBox *m_withdrawAmount;
    std::function<void()> m_onDataChanged;

    QWidget *buildTransactionForm(bool deposit);
    void performDeposit();
    void performWithdrawal();
};

class UsersPage : public QWidget
{
public:
    explicit UsersPage(BankService *service, const UserRecord &user, QWidget *parent = nullptr);
    void refresh();

private:
    BankService *m_service;
    UserRecord m_user;
    QTableWidget *m_table;
    QLineEdit *m_filterEdit;

    QString selectedUsername() const;
    void populate(const QVector<UserRecord> &users);
    void addUser();
    void editUser();
    void deleteUser();
};
