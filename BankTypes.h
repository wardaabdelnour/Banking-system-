#pragma once

#include <QString>

namespace BankPermissions {
constexpr int All = -1;
constexpr int ListClients = 1;
constexpr int AddClient = 2;
constexpr int DeleteClient = 4;
constexpr int UpdateClient = 8;
constexpr int FindClient = 16;
constexpr int Transactions = 32;
constexpr int ManageUsers = 64;
constexpr int AllDefined = ListClients | AddClient | DeleteClient | UpdateClient | FindClient | Transactions | ManageUsers;
}

struct ClientRecord {
    QString accountNumber;
    QString pinCode;
    QString name;
    double balance = 0.0;
};

struct UserRecord {
    QString username;
    QString password; // Kept only for authentication/storage; never displayed in the GUI.
    int permissions = 0;
};

inline bool hasFullAccess(int permissions)
{
    return permissions == BankPermissions::All
        || (permissions & BankPermissions::AllDefined) == BankPermissions::AllDefined;
}

inline bool hasPermission(const UserRecord &user, int permission)
{
    return user.permissions == BankPermissions::All
        || (user.permissions & permission) == permission;
}
