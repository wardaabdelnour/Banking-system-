#include "Pages.h"
#include "UiHelpers.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {
QString moneyText(double value)
{
    return QStringLiteral("EGP %1").arg(value, 0, 'f', 2);
}

QTableWidgetItem *plainItem(const QString &text)
{
    auto *item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

bool clientDialog(QWidget *parent, ClientRecord *client, bool editing)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(editing ? QStringLiteral("Update client") : QStringLiteral("Add new client"));
    dialog.setMinimumWidth(420);

    auto *layout = new QVBoxLayout(&dialog);
    auto *title = new QLabel(editing ? QStringLiteral("Client details") : QStringLiteral("New client details"));
    title->setStyleSheet("font-size:16pt;font-weight:700;color:#172B45;");
    layout->addWidget(title);
    auto *form = new QFormLayout;
    form->setVerticalSpacing(12);

    auto *account = new QLineEdit(client->accountNumber);
    account->setPlaceholderText(QStringLiteral("e.g. A154"));
    if (editing) {
        account->setReadOnly(true);
        account->setStyleSheet("background:#EEF1F4;color:#657386;");
    }
    auto *pin = new QLineEdit(client->pinCode);
    pin->setPlaceholderText(QStringLiteral("Client PIN"));
    pin->setEchoMode(QLineEdit::Password);
    auto *name = new QLineEdit(client->name);
    name->setPlaceholderText(QStringLiteral("Full name"));
    auto *balance = new QDoubleSpinBox;
    balance->setRange(0.0, 1000000000000.0);
    balance->setDecimals(2);
    balance->setValue(client->balance);
    balance->setSuffix(QStringLiteral(" EGP"));

    form->addRow(QStringLiteral("Account number"), account);
    form->addRow(QStringLiteral("PIN code"), pin);
    form->addRow(QStringLiteral("Client name"), name);
    form->addRow(QStringLiteral("Balance"), balance);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (account->text().trimmed().isEmpty() || pin->text().trimmed().isEmpty()
            || name->text().trimmed().isEmpty()) {
            showError(&dialog, QStringLiteral("Please complete all fields."));
            return;
        }
        client->accountNumber = account->text().trimmed();
        client->pinCode = pin->text();
        client->name = name->text().trimmed();
        client->balance = balance->value();
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    return dialog.exec() == QDialog::Accepted;
}

struct PermissionCheck {
    int bit;
    QString label;
};

const QVector<PermissionCheck> permissionChecks = {
    {BankPermissions::ListClients, QStringLiteral("List clients")},
    {BankPermissions::AddClient, QStringLiteral("Add clients")},
    {BankPermissions::DeleteClient, QStringLiteral("Delete clients")},
    {BankPermissions::UpdateClient, QStringLiteral("Update clients")},
    {BankPermissions::FindClient, QStringLiteral("Find clients")},
    {BankPermissions::Transactions, QStringLiteral("Transactions")},
    {BankPermissions::ManageUsers, QStringLiteral("Manage users")}
};

bool userDialog(QWidget *parent, UserRecord *user, bool editing)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(editing ? QStringLiteral("Update user") : QStringLiteral("Add new user"));
    dialog.setMinimumWidth(450);

    auto *layout = new QVBoxLayout(&dialog);
    auto *title = new QLabel(editing ? QStringLiteral("User access settings") : QStringLiteral("Create a user account"));
    title->setStyleSheet("font-size:16pt;font-weight:700;color:#172B45;");
    layout->addWidget(title);

    auto *form = new QFormLayout;
    auto *username = new QLineEdit(user->username);
    if (editing) {
        username->setReadOnly(true);
        username->setStyleSheet("background:#EEF1F4;color:#657386;");
    }
    auto *password = new QLineEdit;
    password->setEchoMode(QLineEdit::Password);
    password->setPlaceholderText(editing ? QStringLiteral("Leave blank to keep current password") : QStringLiteral("Create a password"));
    form->addRow(QStringLiteral("Username"), username);
    form->addRow(QStringLiteral("Password"), password);
    layout->addLayout(form);

    auto *permissionsBox = new QGroupBox(QStringLiteral("Permissions"));
    auto *permissionsLayout = new QVBoxLayout(permissionsBox);
    auto *fullAccess = new QCheckBox(QStringLiteral("Full access"));
    fullAccess->setChecked(hasFullAccess(user->permissions));
    permissionsLayout->addWidget(fullAccess);

    QVector<QCheckBox *> checks;
    for (const PermissionCheck &permission : permissionChecks) {
        auto *check = new QCheckBox(permission.label);
        check->setChecked(hasFullAccess(user->permissions)
                          || (user->permissions & permission.bit) == permission.bit);
        permissionsLayout->addWidget(check);
        checks.push_back(check);
    }
    auto updatePermissionControls = [fullAccess, checks](bool full) {
        for (QCheckBox *check : checks) {
            check->setEnabled(!full);
            check->setChecked(full);
        }
    };
    updatePermissionControls(fullAccess->isChecked());
    QObject::connect(fullAccess, &QCheckBox::toggled, &dialog, updatePermissionControls);
    layout->addWidget(permissionsBox);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (username->text().trimmed().isEmpty()) {
            showError(&dialog, QStringLiteral("Username is required."));
            return;
        }
        if (!editing && password->text().isEmpty()) {
            showError(&dialog, QStringLiteral("Password is required for a new user."));
            return;
        }
        user->username = username->text().trimmed();
        user->password = password->text();
        if (fullAccess->isChecked()) {
            user->permissions = BankPermissions::All;
        } else {
            int value = 0;
            for (int i = 0; i < permissionChecks.size(); ++i) {
                if (checks.at(i)->isChecked())
                    value |= permissionChecks.at(i).bit;
            }
            user->permissions = value;
        }
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    return dialog.exec() == QDialog::Accepted;
}
}

// ---------------- Dashboard ----------------
DashboardPage::DashboardPage(BankService *service, const UserRecord &user, QWidget *parent)
    : QWidget(parent), m_service(service), m_user(user), m_clientCountValue(nullptr),
      m_balanceValue(nullptr), m_accessValue(nullptr)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 26, 28, 26);
    layout->setSpacing(12);
    makePageTitle(QStringLiteral("Overview"), QStringLiteral("A clear view of your banking workspace."), layout);

    auto *welcome = new QLabel(QStringLiteral("Good day, %1").arg(m_user.username));
    welcome->setStyleSheet("font-size:13pt;font-weight:600;color:#35485F;");
    layout->addWidget(welcome);

    auto *cards = new QGridLayout;
    cards->setSpacing(14);
    auto makeMetric = [](const QString &caption, QLabel **value) {
        auto *card = makeCard();
        auto *v = new QVBoxLayout(card);
        v->setContentsMargins(20, 18, 20, 18);
        auto *label = new QLabel(caption);
        label->setObjectName("CardTitle");
        *value = new QLabel(QStringLiteral("—"));
        (*value)->setObjectName("CardValue");
        v->addWidget(label);
        v->addWidget(*value);
        return card;
    };
    cards->addWidget(makeMetric(QStringLiteral("TOTAL CLIENTS"), &m_clientCountValue), 0, 0);
    cards->addWidget(makeMetric(QStringLiteral("TOTAL BALANCES"), &m_balanceValue), 0, 1);
    cards->addWidget(makeMetric(QStringLiteral("YOUR ACCESS"), &m_accessValue), 0, 2);
    layout->addLayout(cards);

    auto *note = makeCard();
    auto *noteLayout = new QVBoxLayout(note);
    noteLayout->setContentsMargins(20, 18, 20, 18);
    auto *noteTitle = new QLabel(QStringLiteral("Workspace information"));
    noteTitle->setStyleSheet("font-size:12pt;font-weight:700;color:#172B45;");
    auto *noteText = new QLabel(QStringLiteral("Client and user records are read from your existing text files. Changes made in this application are saved back to those files using the same #//# record format."));
    noteText->setWordWrap(true);
    noteText->setObjectName("Muted");
    noteLayout->addWidget(noteTitle);
    noteLayout->addWidget(noteText);
    layout->addWidget(note);
    layout->addStretch();
    refresh();
}

void DashboardPage::refresh()
{
    QVector<ClientRecord> clients;
    double total = 0.0;
    QString error;
    if (hasPermission(m_user, BankPermissions::ListClients)
        && m_service->listClients(m_user, &clients, &error)) {
        double sum = 0.0;
        for (const ClientRecord &client : clients) sum += client.balance;
        m_clientCountValue->setText(QString::number(clients.size()));
        m_balanceValue->setText(moneyText(sum));
    } else if (hasPermission(m_user, BankPermissions::Transactions)
               && m_service->totalBalances(m_user, &clients, &total, &error)) {
        m_clientCountValue->setText(QString::number(clients.size()));
        m_balanceValue->setText(moneyText(total));
    } else {
        m_clientCountValue->setText(QStringLiteral("Restricted"));
        m_balanceValue->setText(QStringLiteral("Restricted"));
    }
    m_accessValue->setText(hasFullAccess(m_user.permissions)
                           ? QStringLiteral("Full") : QStringLiteral("Limited"));
    m_accessValue->setStyleSheet("font-size:18pt;font-weight:700;color:#9A8459;");
}

// ---------------- Clients ----------------
ClientsPage::ClientsPage(BankService *service, const UserRecord &user, QWidget *parent)
    : QWidget(parent), m_service(service), m_user(user), m_table(nullptr), m_searchEdit(nullptr),
      m_addButton(nullptr), m_editButton(nullptr), m_deleteButton(nullptr), m_findButton(nullptr)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 26, 28, 26);
    layout->setSpacing(12);
    makePageTitle(QStringLiteral("Client management"), QStringLiteral("Maintain client records and account information."), layout);

    auto *toolbar = new QHBoxLayout;
    if (hasPermission(m_user, BankPermissions::AddClient)) {
        m_addButton = new QPushButton(QStringLiteral("＋  Add client"));
        m_addButton->setObjectName("PrimaryButton");
        toolbar->addWidget(m_addButton);
        QObject::connect(m_addButton, &QPushButton::clicked, this, [this] { addClient(); });
    }
    if (hasPermission(m_user, BankPermissions::UpdateClient)) {
        m_editButton = new QPushButton(QStringLiteral("Edit selected"));
        toolbar->addWidget(m_editButton);
        QObject::connect(m_editButton, &QPushButton::clicked, this, [this] { editClient(); });
    }
    if (hasPermission(m_user, BankPermissions::DeleteClient)) {
        m_deleteButton = new QPushButton(QStringLiteral("Delete selected"));
        m_deleteButton->setObjectName("DangerButton");
        toolbar->addWidget(m_deleteButton);
        QObject::connect(m_deleteButton, &QPushButton::clicked, this, [this] { deleteClient(); });
    }
    toolbar->addStretch();
    if (hasPermission(m_user, BankPermissions::FindClient)
        || hasPermission(m_user, BankPermissions::UpdateClient)
        || hasPermission(m_user, BankPermissions::DeleteClient)) {
        m_searchEdit = new QLineEdit;
        m_searchEdit->setPlaceholderText(QStringLiteral("Account number"));
        m_searchEdit->setMaximumWidth(190);
        toolbar->addWidget(m_searchEdit);
        m_findButton = new QPushButton(QStringLiteral("Find / select"));
        toolbar->addWidget(m_findButton);
        QObject::connect(m_findButton, &QPushButton::clicked, this, [this] { findClient(); });
        QObject::connect(m_searchEdit, &QLineEdit::returnPressed, this, [this] { findClient(); });
    }
    auto *refreshButton = new QPushButton(QStringLiteral("Refresh"));
    toolbar->addWidget(refreshButton);
    QObject::connect(refreshButton, &QPushButton::clicked, this, [this] { refresh(); });
    layout->addLayout(toolbar);

    auto *panel = makeCard();
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(14, 14, 14, 14);
    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({QStringLiteral("ACCOUNT NUMBER"), QStringLiteral("CLIENT NAME"), QStringLiteral("BALANCE (EGP)")});
    configureTable(m_table);
    panelLayout->addWidget(m_table);
    layout->addWidget(panel, 1);

    auto *footnote = new QLabel(QStringLiteral("PIN codes are intentionally hidden in the client list."));
    footnote->setObjectName("Muted");
    layout->addWidget(footnote);

    if (hasPermission(m_user, BankPermissions::ListClients))
        refresh();
    else {
        const bool canSelectByAccount = hasPermission(m_user, BankPermissions::FindClient)
            || hasPermission(m_user, BankPermissions::UpdateClient)
            || hasPermission(m_user, BankPermissions::DeleteClient);
        auto *hint = new QLabel(canSelectByAccount
            ? QStringLiteral("Use the account search above to select a client for an allowed action.")
            : QStringLiteral("Use the Add client action to create a new account."));
        hint->setObjectName("Muted");
        layout->addWidget(hint);
    }
}

QString ClientsPage::selectedAccountNumber() const
{
    const int row = m_table->currentRow();
    if (row < 0 || !m_table->item(row, 0)) return QString();
    return m_table->item(row, 0)->text();
}

void ClientsPage::populate(const QVector<ClientRecord> &clients)
{
    m_table->setRowCount(0);
    for (const ClientRecord &client : clients) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, plainItem(client.accountNumber));
        m_table->setItem(row, 1, plainItem(client.name));
        auto *balance = plainItem(QString::number(client.balance, 'f', 2));
        balance->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 2, balance);
        if (row == 0)
            m_table->setCurrentCell(0, 0);
    }
}

void ClientsPage::refresh()
{
    if (!hasPermission(m_user, BankPermissions::ListClients)) {
        m_table->setRowCount(0);
        return;
    }
    QVector<ClientRecord> clients;
    QString error;
    if (!m_service->listClients(m_user, &clients, &error)) {
        showError(this, error);
        return;
    }
    populate(clients);
}

void ClientsPage::addClient()
{
    ClientRecord client;
    if (!clientDialog(this, &client, false)) return;
    QString error;
    if (!m_service->addClient(m_user, client, &error)) {
        showError(this, error);
        return;
    }
    showInfo(this, QStringLiteral("The client record has been added."));
    refresh();
    if (m_onDataChanged) m_onDataChanged();
}

void ClientsPage::editClient()
{
    const QString account = selectedAccountNumber();
    if (account.isEmpty()) {
        showError(this, QStringLiteral("Select a client first."));
        return;
    }
    ClientRecord client;
    QString error;
    if (!m_service->getClientForAction(m_user, account, BankPermissions::UpdateClient, &client, &error)) {
        showError(this, error);
        return;
    }
    if (!clientDialog(this, &client, true)) return;
    if (!m_service->updateClient(m_user, account, client, &error)) {
        showError(this, error);
        return;
    }
    showInfo(this, QStringLiteral("The client record has been updated."));
    refresh();
    if (m_onDataChanged) m_onDataChanged();
}

void ClientsPage::deleteClient()
{
    const QString account = selectedAccountNumber();
    if (account.isEmpty()) {
        showError(this, QStringLiteral("Select a client first."));
        return;
    }
    if (QMessageBox::question(this, QStringLiteral("Confirm deletion"),
                              QStringLiteral("Delete client account %1? This cannot be undone.").arg(account),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;
    QString error;
    if (!m_service->deleteClient(m_user, account, &error)) {
        showError(this, error);
        return;
    }
    showInfo(this, QStringLiteral("The client record has been deleted."));
    refresh();
    if (m_onDataChanged) m_onDataChanged();
}

void ClientsPage::findClient()
{
    const QString account = m_searchEdit ? m_searchEdit->text().trimmed() : QString();
    if (account.isEmpty()) {
        showError(this, QStringLiteral("Enter an account number to search."));
        return;
    }
    ClientRecord client;
    QString error;
    bool found = false;
    if (hasPermission(m_user, BankPermissions::FindClient))
        found = m_service->findClient(m_user, account, &client, &error);
    else if (hasPermission(m_user, BankPermissions::UpdateClient))
        found = m_service->getClientForAction(m_user, account, BankPermissions::UpdateClient, &client, &error);
    else if (hasPermission(m_user, BankPermissions::DeleteClient))
        found = m_service->getClientForAction(m_user, account, BankPermissions::DeleteClient, &client, &error);
    if (!found) {
        showError(this, error);
        return;
    }
    populate({client});
}

// ---------------- Transactions ----------------
TransactionsPage::TransactionsPage(BankService *service, const UserRecord &user, QWidget *parent)
    : QWidget(parent), m_service(service), m_user(user), m_balancesTable(nullptr),
      m_depositAccount(nullptr), m_withdrawAccount(nullptr), m_depositAmount(nullptr), m_withdrawAmount(nullptr)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 26, 28, 26);
    layout->setSpacing(12);
    makePageTitle(QStringLiteral("Transactions"), QStringLiteral("Deposit, withdraw, and review account balances."), layout);

    auto *tabs = new QTabWidget;
    tabs->addTab(buildTransactionForm(true), QStringLiteral("Deposit"));
    tabs->addTab(buildTransactionForm(false), QStringLiteral("Withdraw"));

    auto *balancesPage = new QWidget;
    auto *balancesLayout = new QVBoxLayout(balancesPage);
    balancesLayout->setContentsMargins(14, 14, 14, 14);
    auto *refreshButton = new QPushButton(QStringLiteral("Refresh balances"));
    refreshButton->setObjectName("PrimaryButton");
    QObject::connect(refreshButton, &QPushButton::clicked, this, [this] { refreshBalances(); });
    balancesLayout->addWidget(refreshButton, 0, Qt::AlignRight);
    m_balancesTable = new QTableWidget(0, 3);
    m_balancesTable->setHorizontalHeaderLabels({QStringLiteral("ACCOUNT NUMBER"), QStringLiteral("CLIENT NAME"), QStringLiteral("BALANCE (EGP)")});
    configureTable(m_balancesTable);
    balancesLayout->addWidget(m_balancesTable, 1);
    auto *totalLabel = new QLabel;
    totalLabel->setObjectName("Muted");
    totalLabel->setProperty("isTotalLabel", true);
    totalLabel->setStyleSheet("font-size:14pt;font-weight:700;color:#172B45;");
    balancesLayout->addWidget(totalLabel);
    tabs->addTab(balancesPage, QStringLiteral("Total balances"));
    QObject::connect(tabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 2) refreshBalances();
    });
    layout->addWidget(tabs, 1);
}

QWidget *TransactionsPage::buildTransactionForm(bool deposit)
{
    auto *page = new QWidget;
    auto *outer = new QVBoxLayout(page);
    outer->setContentsMargins(22, 22, 22, 22);
    outer->addStretch();

    auto *card = makeCard();
    card->setMaximumWidth(560);
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(28, 26, 28, 26);
    layout->setSpacing(14);
    auto *heading = new QLabel(deposit ? QStringLiteral("Make a deposit") : QStringLiteral("Make a withdrawal"));
    heading->setStyleSheet("font-size:17pt;font-weight:700;color:#172B45;");
    layout->addWidget(heading);
    auto *description = new QLabel(deposit
        ? QStringLiteral("Add funds to an existing client account.")
        : QStringLiteral("Withdraw funds within the available account balance."));
    description->setObjectName("Muted");
    layout->addWidget(description);

    auto *form = new QFormLayout;
    auto *account = new QLineEdit;
    account->setPlaceholderText(QStringLiteral("Enter account number"));
    auto *amount = new QDoubleSpinBox;
    amount->setRange(0.01, 1000000000000.0);
    amount->setDecimals(2);
    amount->setPrefix(QStringLiteral("EGP "));
    amount->setSingleStep(100.0);
    form->addRow(QStringLiteral("Account number"), account);
    form->addRow(deposit ? QStringLiteral("Deposit amount") : QStringLiteral("Withdrawal amount"), amount);
    layout->addLayout(form);

    auto *button = new QPushButton(deposit ? QStringLiteral("Confirm deposit") : QStringLiteral("Confirm withdrawal"));
    button->setObjectName("GoldButton");
    button->setMinimumHeight(42);
    layout->addWidget(button);
    outer->addWidget(card, 0, Qt::AlignHCenter);
    outer->addStretch();

    if (deposit) {
        m_depositAccount = account;
        m_depositAmount = amount;
        QObject::connect(button, &QPushButton::clicked, this, [this] { performDeposit(); });
    } else {
        m_withdrawAccount = account;
        m_withdrawAmount = amount;
        QObject::connect(button, &QPushButton::clicked, this, [this] { performWithdrawal(); });
    }
    return page;
}

void TransactionsPage::performDeposit()
{
    const QString account = m_depositAccount->text().trimmed();
    const double amount = m_depositAmount->value();
    if (account.isEmpty()) { showError(this, QStringLiteral("Enter an account number.")); return; }
    if (QMessageBox::question(this, QStringLiteral("Confirm deposit"),
        QStringLiteral("Deposit %1 to account %2?").arg(moneyText(amount), account),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;

    QString error;
    double balance = 0.0;
    if (!m_service->deposit(m_user, account, amount, &balance, &error)) {
        showError(this, error);
        return;
    }
    showInfo(this, QStringLiteral("Deposit completed.\nNew balance: %1").arg(moneyText(balance)));
    m_depositAmount->setValue(0.01);
    refreshBalances();
    if (m_onDataChanged) m_onDataChanged();
}

void TransactionsPage::performWithdrawal()
{
    const QString account = m_withdrawAccount->text().trimmed();
    const double amount = m_withdrawAmount->value();
    if (account.isEmpty()) { showError(this, QStringLiteral("Enter an account number.")); return; }
    if (QMessageBox::question(this, QStringLiteral("Confirm withdrawal"),
        QStringLiteral("Withdraw %1 from account %2?").arg(moneyText(amount), account),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;

    QString error;
    double balance = 0.0;
    if (!m_service->withdraw(m_user, account, amount, &balance, &error)) {
        showError(this, error);
        return;
    }
    showInfo(this, QStringLiteral("Withdrawal completed.\nNew balance: %1").arg(moneyText(balance)));
    m_withdrawAmount->setValue(0.01);
    refreshBalances();
    if (m_onDataChanged) m_onDataChanged();
}

void TransactionsPage::refreshBalances()
{
    QVector<ClientRecord> clients;
    double total = 0.0;
    QString error;
    if (!m_service->totalBalances(m_user, &clients, &total, &error)) {
        showError(this, error);
        return;
    }
    m_balancesTable->setRowCount(0);
    for (const ClientRecord &client : clients) {
        const int row = m_balancesTable->rowCount();
        m_balancesTable->insertRow(row);
        m_balancesTable->setItem(row, 0, plainItem(client.accountNumber));
        m_balancesTable->setItem(row, 1, plainItem(client.name));
        m_balancesTable->setItem(row, 2, plainItem(QString::number(client.balance, 'f', 2)));
    }
    // Keep the total label in the same balances page, below the table.
    auto *page = qobject_cast<QWidget *>(m_balancesTable->parentWidget());
    if (page && page->layout()) {
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->property("isTotalLabel").toBool())
                label->setText(QStringLiteral("Total balances: %1").arg(moneyText(total)));
        }
    }
}

// ---------------- User management ----------------
UsersPage::UsersPage(BankService *service, const UserRecord &user, QWidget *parent)
    : QWidget(parent), m_service(service), m_user(user), m_table(nullptr), m_filterEdit(nullptr)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 26, 28, 26);
    layout->setSpacing(12);
    makePageTitle(QStringLiteral("User management"), QStringLiteral("Manage staff accounts and access permissions."), layout);

    auto *toolbar = new QHBoxLayout;
    auto *addButton = new QPushButton(QStringLiteral("＋  Add user"));
    addButton->setObjectName("PrimaryButton");
    auto *editButton = new QPushButton(QStringLiteral("Edit selected"));
    auto *deleteButton = new QPushButton(QStringLiteral("Delete selected"));
    deleteButton->setObjectName("DangerButton");
    m_filterEdit = new QLineEdit;
    m_filterEdit->setPlaceholderText(QStringLiteral("Filter by username"));
    m_filterEdit->setMaximumWidth(220);
    auto *refreshButton = new QPushButton(QStringLiteral("Refresh"));
    toolbar->addWidget(addButton);
    toolbar->addWidget(editButton);
    toolbar->addWidget(deleteButton);
    toolbar->addStretch();
    toolbar->addWidget(m_filterEdit);
    toolbar->addWidget(refreshButton);
    layout->addLayout(toolbar);

    auto *panel = makeCard();
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(14, 14, 14, 14);
    m_table = new QTableWidget(0, 2);
    m_table->setHorizontalHeaderLabels({QStringLiteral("USERNAME"), QStringLiteral("PERMISSIONS")});
    configureTable(m_table);
    panelLayout->addWidget(m_table);
    layout->addWidget(panel, 1);

    auto *note = new QLabel(QStringLiteral("Passwords are never displayed. Leave the password field empty when updating a user to keep the current password."));
    note->setObjectName("Muted");
    note->setWordWrap(true);
    layout->addWidget(note);

    QObject::connect(addButton, &QPushButton::clicked, this, [this] { addUser(); });
    QObject::connect(editButton, &QPushButton::clicked, this, [this] { editUser(); });
    QObject::connect(deleteButton, &QPushButton::clicked, this, [this] { deleteUser(); });
    QObject::connect(refreshButton, &QPushButton::clicked, this, [this] { refresh(); });
    QObject::connect(m_filterEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        for (int row = 0; row < m_table->rowCount(); ++row) {
            const bool match = m_table->item(row, 0)->text().contains(text, Qt::CaseInsensitive);
            m_table->setRowHidden(row, !match);
        }
    });
    refresh();
}

QString UsersPage::selectedUsername() const
{
    const int row = m_table->currentRow();
    if (row < 0 || !m_table->item(row, 0)) return QString();
    return m_table->item(row, 0)->text();
}

void UsersPage::populate(const QVector<UserRecord> &users)
{
    m_table->setRowCount(0);
    for (const UserRecord &user : users) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, plainItem(user.username));
        m_table->setItem(row, 1, plainItem(permissionSummary(user.permissions)));
    }
}

void UsersPage::refresh()
{
    if (!hasPermission(m_user, BankPermissions::ManageUsers))
        return;
    QVector<UserRecord> users;
    QString error;
    if (!m_service->listUsers(m_user, &users, &error)) {
        showError(this, error);
        return;
    }
    populate(users);
}

void UsersPage::addUser()
{
    UserRecord user;
    if (!userDialog(this, &user, false)) return;
    QString error;
    if (!m_service->addUser(m_user, user, &error)) {
        showError(this, error);
        return;
    }
    showInfo(this, QStringLiteral("The user account has been created."));
    refresh();
}

void UsersPage::editUser()
{
    const QString username = selectedUsername();
    if (username.isEmpty()) { showError(this, QStringLiteral("Select a user first.")); return; }
    QVector<UserRecord> users;
    QString error;
    if (!m_service->listUsers(m_user, &users, &error)) { showError(this, error); return; }
    UserRecord current;
    bool found = false;
    for (const UserRecord &user : users) {
        if (user.username == username) { current = user; found = true; break; }
    }
    if (!found) { showError(this, QStringLiteral("User not found.")); return; }
    UserRecord updated = current;
    if (!userDialog(this, &updated, true)) return;
    if (!m_service->updateUser(m_user, username, updated.password, updated.permissions, &error)) {
        showError(this, error);
        return;
    }
    showInfo(this, QStringLiteral("User settings have been updated."));
    refresh();
}

void UsersPage::deleteUser()
{
    const QString username = selectedUsername();
    if (username.isEmpty()) { showError(this, QStringLiteral("Select a user first.")); return; }
    if (QMessageBox::question(this, QStringLiteral("Confirm deletion"),
                              QStringLiteral("Delete user '%1'? This cannot be undone.").arg(username),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;
    QString error;
    if (!m_service->deleteUser(m_user, username, &error)) { showError(this, error); return; }
    showInfo(this, QStringLiteral("The user account has been deleted."));
    refresh();
}
