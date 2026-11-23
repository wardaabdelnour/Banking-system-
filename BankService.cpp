#include "BankService.h"

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QStringList>
#include <QTextStream>
#include <QStringConverter>

#include <cmath>

namespace {
const QString Separator = QStringLiteral("#//#");

QString locateDataDirectory(const QString &requested)
{
    if (!requested.trimmed().isEmpty())
        return QDir(requested).absolutePath();

    QStringList starts;
    starts << QDir::currentPath() << QCoreApplication::applicationDirPath();

    for (const QString &start : starts) {
        QDir dir(start);
        for (int depth = 0; depth < 6; ++depth) {
            const bool hasClients = QFileInfo::exists(dir.filePath("Bank.txt"));
            const bool hasUsers = QFileInfo::exists(dir.filePath("users.txt"));
            if (hasClients && hasUsers)
                return dir.absolutePath();
            if (!dir.cdUp())
                break;
        }
    }

    return QDir::currentPath();
}

bool parseDouble(const QString &value, double *out)
{
    bool ok = false;
    const double number = value.toDouble(&ok);
    if (!ok || !std::isfinite(number))
        return false;
    *out = number;
    return true;
}
}

BankService::BankService(const QString &dataDirectory)
    : m_dataDirectory(locateDataDirectory(dataDirectory))
{
}

QString BankService::clientFilePath() const
{
    return QDir(m_dataDirectory).filePath("Bank.txt");
}

QString BankService::userFilePath() const
{
    return QDir(m_dataDirectory).filePath("users.txt");
}

bool BankService::checkPermission(const UserRecord &actor, int permission, QString *error) const
{
    if (hasPermission(actor, permission))
        return true;
    if (error)
        *error = QStringLiteral("Access denied. Your account does not have permission for this operation.");
    return false;
}

QVector<ClientRecord> BankService::readClients(QString *error) const
{
    if (error) error->clear();
    QVector<ClientRecord> result;
    QFile file(clientFilePath());
    if (!file.exists()) {
        if (error)
            *error = QStringLiteral("Bank.txt was not found in:\n%1").arg(m_dataDirectory);
        return result;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error)
            *error = QStringLiteral("Cannot open Bank.txt: %1").arg(file.errorString());
        return result;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    int lineNumber = 0;
    while (!in.atEnd()) {
        const QString line = in.readLine();
        ++lineNumber;
        if (line.trimmed().isEmpty())
            continue;

        const QStringList fields = line.split(Separator);
        if (fields.size() != 4) {
            if (error)
                *error = QStringLiteral("Invalid record in Bank.txt at line %1.").arg(lineNumber);
            return {};
        }

        ClientRecord client;
        client.accountNumber = fields.at(0);
        client.pinCode = fields.at(1);
        client.name = fields.at(2);
        if (!parseDouble(fields.at(3), &client.balance)) {
            if (error)
                *error = QStringLiteral("Invalid balance in Bank.txt at line %1.").arg(lineNumber);
            return {};
        }
        result.push_back(client);
    }
    return result;
}

QVector<UserRecord> BankService::readUsers(QString *error) const
{
    if (error) error->clear();
    QVector<UserRecord> result;
    QFile file(userFilePath());
    if (!file.exists()) {
        if (error)
            *error = QStringLiteral("users.txt was not found in:\n%1").arg(m_dataDirectory);
        return result;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error)
            *error = QStringLiteral("Cannot open users.txt: %1").arg(file.errorString());
        return result;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    int lineNumber = 0;
    while (!in.atEnd()) {
        const QString line = in.readLine();
        ++lineNumber;
        if (line.trimmed().isEmpty())
            continue;

        const QStringList fields = line.split(Separator);
        bool ok = false;
        if (fields.size() != 3) {
            if (error)
                *error = QStringLiteral("Invalid record in users.txt at line %1.").arg(lineNumber);
            return {};
        }

        UserRecord user;
        user.username = fields.at(0);
        user.password = fields.at(1);
        user.permissions = fields.at(2).toInt(&ok);
        if (!ok) {
            if (error)
                *error = QStringLiteral("Invalid permissions in users.txt at line %1.").arg(lineNumber);
            return {};
        }
        result.push_back(user);
    }
    return result;
}

bool BankService::writeClients(const QVector<ClientRecord> &clients, QString *error) const
{
    QSaveFile file(clientFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error)
            *error = QStringLiteral("Cannot write Bank.txt: %1").arg(file.errorString());
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    for (const ClientRecord &client : clients) {
        out << client.accountNumber << Separator
            << client.pinCode << Separator
            << client.name << Separator
            << QString::number(client.balance, 'f', 6) << '\n';
    }
    out.flush();
    if (out.status() != QTextStream::Ok || !file.commit()) {
        if (error)
            *error = QStringLiteral("Could not safely save Bank.txt.");
        return false;
    }
    return true;
}

bool BankService::writeUsers(const QVector<UserRecord> &users, QString *error) const
{
    QSaveFile file(userFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error)
            *error = QStringLiteral("Cannot write users.txt: %1").arg(file.errorString());
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    for (const UserRecord &user : users) {
        out << user.username << Separator
            << user.password << Separator
            << user.permissions << '\n';
    }
    out.flush();
    if (out.status() != QTextStream::Ok || !file.commit()) {
        if (error)
            *error = QStringLiteral("Could not safely save users.txt.");
        return false;
    }
    return true;
}

bool BankService::authenticate(const QString &username, const QString &password,
                               UserRecord *authenticatedUser, QString *error) const
{
    const QVector<UserRecord> allUsers = readUsers(error);
    if (error && !error->isEmpty())
        return false;
    for (const UserRecord &user : allUsers) {
        if (user.username == username && user.password == password) {
            if (authenticatedUser)
                *authenticatedUser = user;
            return true;
        }
    }
    if (error)
        *error = QStringLiteral("The username or password is incorrect.");
    return false;
}

bool BankService::listClients(const UserRecord &actor, QVector<ClientRecord> *result, QString *error) const
{
    if (!checkPermission(actor, BankPermissions::ListClients, error))
        return false;
    const QVector<ClientRecord> clients = readClients(error);
    if (error && !error->isEmpty())
        return false;
    if (result)
        *result = clients;
    return true;
}

bool BankService::findClient(const UserRecord &actor, const QString &accountNumber,
                             ClientRecord *result, QString *error) const
{
    if (!checkPermission(actor, BankPermissions::FindClient, error))
        return false;
    const QVector<ClientRecord> clients = readClients(error);
    if (error && !error->isEmpty())
        return false;
    for (const ClientRecord &client : clients) {
        if (client.accountNumber == accountNumber) {
            if (result)
                *result = client;
            return true;
        }
    }
    if (error)
        *error = QStringLiteral("No client was found with this account number.");
    return false;
}

bool BankService::getClientForAction(const UserRecord &actor, const QString &accountNumber,
                                     int requiredPermission, ClientRecord *result, QString *error) const
{
    if (!checkPermission(actor, requiredPermission, error))
        return false;
    const QVector<ClientRecord> clients = readClients(error);
    if (error && !error->isEmpty())
        return false;
    for (const ClientRecord &client : clients) {
        if (client.accountNumber == accountNumber) {
            if (result) *result = client;
            return true;
        }
    }
    if (error) *error = QStringLiteral("Client not found.");
    return false;
}

bool BankService::addClient(const UserRecord &actor, const ClientRecord &client, QString *error)
{
    if (!checkPermission(actor, BankPermissions::AddClient, error))
        return false;
    if (client.accountNumber.trimmed().isEmpty() || client.pinCode.trimmed().isEmpty()
        || client.name.trimmed().isEmpty()) {
        if (error) *error = QStringLiteral("Please complete all client fields.");
        return false;
    }
    if (client.balance < 0.0 || !std::isfinite(client.balance)) {
        if (error) *error = QStringLiteral("The opening balance cannot be negative or invalid.");
        return false;
    }

    QVector<ClientRecord> clients = readClients(error);
    if (error && !error->isEmpty()) return false;
    for (const ClientRecord &existing : clients) {
        if (existing.accountNumber == client.accountNumber) {
            if (error) *error = QStringLiteral("This account number already exists.");
            return false;
        }
    }
    clients.push_back(client);
    return writeClients(clients, error);
}

bool BankService::updateClient(const UserRecord &actor, const QString &accountNumber,
                               const ClientRecord &updated, QString *error)
{
    if (!checkPermission(actor, BankPermissions::UpdateClient, error))
        return false;
    if (updated.pinCode.trimmed().isEmpty() || updated.name.trimmed().isEmpty()
        || updated.balance < 0.0 || !std::isfinite(updated.balance)) {
        if (error) *error = QStringLiteral("Enter valid client details and a non-negative balance.");
        return false;
    }
    QVector<ClientRecord> clients = readClients(error);
    if (error && !error->isEmpty()) return false;
    for (ClientRecord &client : clients) {
        if (client.accountNumber == accountNumber) {
            client.pinCode = updated.pinCode;
            client.name = updated.name;
            client.balance = updated.balance;
            return writeClients(clients, error);
        }
    }
    if (error) *error = QStringLiteral("Client not found.");
    return false;
}

bool BankService::deleteClient(const UserRecord &actor, const QString &accountNumber, QString *error)
{
    if (!checkPermission(actor, BankPermissions::DeleteClient, error))
        return false;
    QVector<ClientRecord> clients = readClients(error);
    if (error && !error->isEmpty()) return false;
    for (int i = 0; i < clients.size(); ++i) {
        if (clients.at(i).accountNumber == accountNumber) {
            clients.removeAt(i);
            return writeClients(clients, error);
        }
    }
    if (error) *error = QStringLiteral("Client not found.");
    return false;
}

bool BankService::deposit(const UserRecord &actor, const QString &accountNumber, double amount,
                          double *newBalance, QString *error)
{
    if (!checkPermission(actor, BankPermissions::Transactions, error)) return false;
    if (amount <= 0.0 || !std::isfinite(amount)) {
        if (error) *error = QStringLiteral("Enter an amount greater than zero.");
        return false;
    }
    QVector<ClientRecord> clients = readClients(error);
    if (error && !error->isEmpty()) return false;
    for (ClientRecord &client : clients) {
        if (client.accountNumber == accountNumber) {
            client.balance += amount;
            if (!writeClients(clients, error)) return false;
            if (newBalance) *newBalance = client.balance;
            return true;
        }
    }
    if (error) *error = QStringLiteral("Client not found.");
    return false;
}

bool BankService::withdraw(const UserRecord &actor, const QString &accountNumber, double amount,
                           double *newBalance, QString *error)
{
    if (!checkPermission(actor, BankPermissions::Transactions, error)) return false;
    if (amount <= 0.0 || !std::isfinite(amount)) {
        if (error) *error = QStringLiteral("Enter an amount greater than zero.");
        return false;
    }
    QVector<ClientRecord> clients = readClients(error);
    if (error && !error->isEmpty()) return false;
    for (ClientRecord &client : clients) {
        if (client.accountNumber == accountNumber) {
            if (amount > client.balance) {
                if (error) *error = QStringLiteral("The withdrawal amount exceeds the available balance.");
                return false;
            }
            client.balance -= amount;
            if (!writeClients(clients, error)) return false;
            if (newBalance) *newBalance = client.balance;
            return true;
        }
    }
    if (error) *error = QStringLiteral("Client not found.");
    return false;
}

bool BankService::totalBalances(const UserRecord &actor, QVector<ClientRecord> *clientsOut,
                                double *total, QString *error) const
{
    if (!checkPermission(actor, BankPermissions::Transactions, error)) return false;
    const QVector<ClientRecord> clients = readClients(error);
    if (error && !error->isEmpty()) return false;
    double sum = 0.0;
    for (const ClientRecord &client : clients)
        sum += client.balance;
    if (clientsOut) *clientsOut = clients;
    if (total) *total = sum;
    return true;
}

bool BankService::listUsers(const UserRecord &actor, QVector<UserRecord> *result, QString *error) const
{
    if (!checkPermission(actor, BankPermissions::ManageUsers, error)) return false;
    const QVector<UserRecord> users = readUsers(error);
    if (error && !error->isEmpty()) return false;
    if (result) *result = users;
    return true;
}

bool BankService::addUser(const UserRecord &actor, const UserRecord &user, QString *error)
{
    if (!checkPermission(actor, BankPermissions::ManageUsers, error)) return false;
    if (user.username.trimmed().isEmpty() || user.password.isEmpty()) {
        if (error) *error = QStringLiteral("Username and password are required.");
        return false;
    }
    QVector<UserRecord> users = readUsers(error);
    if (error && !error->isEmpty()) return false;
    for (const UserRecord &existing : users) {
        if (existing.username.compare(user.username, Qt::CaseSensitive) == 0) {
            if (error) *error = QStringLiteral("This username already exists.");
            return false;
        }
    }
    users.push_back(user);
    return writeUsers(users, error);
}

bool BankService::updateUser(const UserRecord &actor, const QString &username,
                             const QString &newPassword, int newPermissions, QString *error)
{
    if (!checkPermission(actor, BankPermissions::ManageUsers, error)) return false;
    QVector<UserRecord> users = readUsers(error);
    if (error && !error->isEmpty()) return false;
    for (UserRecord &user : users) {
        if (user.username == username) {
            if (!newPassword.isEmpty())
                user.password = newPassword;
            user.permissions = newPermissions;
            return writeUsers(users, error);
        }
    }
    if (error) *error = QStringLiteral("User not found.");
    return false;
}

bool BankService::deleteUser(const UserRecord &actor, const QString &username, QString *error)
{
    if (!checkPermission(actor, BankPermissions::ManageUsers, error)) return false;
    QVector<UserRecord> users = readUsers(error);
    if (error && !error->isEmpty()) return false;
    for (int i = 0; i < users.size(); ++i) {
        if (users.at(i).username == username) {
            users.removeAt(i);
            return writeUsers(users, error);
        }
    }
    if (error) *error = QStringLiteral("User not found.");
    return false;
}
