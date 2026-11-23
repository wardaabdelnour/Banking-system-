#pragma once

#include "BankTypes.h"

#include <QVector>
#include <QString>

class BankService
{
public:
    explicit BankService(const QString &dataDirectory = QString());

    QString dataDirectory() const { return m_dataDirectory; }

    bool authenticate(const QString &username, const QString &password,
                      UserRecord *authenticatedUser, QString *error = nullptr) const;

    bool listClients(const UserRecord &actor, QVector<ClientRecord> *result,
                     QString *error = nullptr) const;
    bool findClient(const UserRecord &actor, const QString &accountNumber,
                    ClientRecord *result, QString *error = nullptr) const;
    bool getClientForAction(const UserRecord &actor, const QString &accountNumber, int requiredPermission,
                            ClientRecord *result, QString *error = nullptr) const;
    bool addClient(const UserRecord &actor, const ClientRecord &client,
                   QString *error = nullptr);
    bool updateClient(const UserRecord &actor, const QString &accountNumber,
                      const ClientRecord &updated, QString *error = nullptr);
    bool deleteClient(const UserRecord &actor, const QString &accountNumber,
                      QString *error = nullptr);

    bool deposit(const UserRecord &actor, const QString &accountNumber, double amount,
                 double *newBalance, QString *error = nullptr);
    bool withdraw(const UserRecord &actor, const QString &accountNumber, double amount,
                  double *newBalance, QString *error = nullptr);
    bool totalBalances(const UserRecord &actor, QVector<ClientRecord> *clients,
                       double *total, QString *error = nullptr) const;

    bool listUsers(const UserRecord &actor, QVector<UserRecord> *result,
                   QString *error = nullptr) const;
    bool addUser(const UserRecord &actor, const UserRecord &user,
                 QString *error = nullptr);
    bool updateUser(const UserRecord &actor, const QString &username,
                    const QString &newPassword, int newPermissions,
                    QString *error = nullptr);
    bool deleteUser(const UserRecord &actor, const QString &username,
                    QString *error = nullptr);

private:
    QString m_dataDirectory;
    QString clientFilePath() const;
    QString userFilePath() const;

    QVector<ClientRecord> readClients(QString *error = nullptr) const;
    QVector<UserRecord> readUsers(QString *error = nullptr) const;
    bool writeClients(const QVector<ClientRecord> &clients, QString *error = nullptr) const;
    bool writeUsers(const QVector<UserRecord> &users, QString *error = nullptr) const;
    bool checkPermission(const UserRecord &actor, int permission, QString *error) const;
};
