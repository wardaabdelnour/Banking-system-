#include "MainWindow.h"
#include "LoginWindow.h"
#include "Pages.h"
#include "UiHelpers.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>

namespace {
QPushButton *makeNavButton(const QString &text)
{
    auto *button = new QPushButton(text);
    button->setObjectName("NavButton");
    button->setMinimumHeight(43);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}
}

MainWindow::MainWindow(BankService *service, const UserRecord &user, QWidget *parent)
    : QMainWindow(parent), m_service(service), m_user(user), m_pages(nullptr),
      m_dashboardPage(nullptr), m_clientsPage(nullptr), m_transactionsPage(nullptr), m_usersPage(nullptr),
      m_dashboardNav(nullptr), m_clientsNav(nullptr), m_transactionsNav(nullptr), m_usersNav(nullptr)
{
    setWindowTitle(QStringLiteral("Meridian Bank | Banking Workspace"));
    setMinimumSize(1100, 720);
    resize(1380, 850);
    setAttribute(Qt::WA_DeleteOnClose, true);

    auto *central = new QWidget;
    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *sidebar = new QFrame;
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(238);
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(18, 26, 18, 20);
    sideLayout->setSpacing(8);

    auto *brand = new QLabel(QStringLiteral("MERIDIAN"));
    brand->setObjectName("BrandTitle");
    auto *brandCaption = new QLabel(QStringLiteral("BANKING WORKSPACE"));
    brandCaption->setObjectName("BrandCaption");
    sideLayout->addWidget(brand);
    sideLayout->addWidget(brandCaption);
    sideLayout->addSpacing(28);

    auto *sectionLabel = new QLabel(QStringLiteral("WORKSPACE"));
    sectionLabel->setStyleSheet("color:#91A0B1;font-size:8pt;font-weight:700;letter-spacing:1px;padding-left:12px;");
    sideLayout->addWidget(sectionLabel);

    m_dashboardNav = makeNavButton(QStringLiteral("◈   Overview"));
    m_clientsNav = makeNavButton(QStringLiteral("▤   Client management"));
    m_transactionsNav = makeNavButton(QStringLiteral("↔   Transactions"));
    m_usersNav = makeNavButton(QStringLiteral("♙   User management"));
    sideLayout->addWidget(m_dashboardNav);

    const int clientPermissions = BankPermissions::ListClients | BankPermissions::AddClient
        | BankPermissions::DeleteClient | BankPermissions::UpdateClient | BankPermissions::FindClient;
    const bool hasClientAccess = m_user.permissions == BankPermissions::All
        || (m_user.permissions & clientPermissions) != 0;
    const bool hasTransactionAccess = hasPermission(m_user, BankPermissions::Transactions);
    const bool hasUserManagementAccess = hasPermission(m_user, BankPermissions::ManageUsers);
    m_clientsNav->setVisible(hasClientAccess);
    m_transactionsNav->setVisible(hasTransactionAccess);
    m_usersNav->setVisible(hasUserManagementAccess);
    if (hasClientAccess) sideLayout->addWidget(m_clientsNav);
    if (hasTransactionAccess) sideLayout->addWidget(m_transactionsNav);
    if (hasUserManagementAccess) sideLayout->addWidget(m_usersNav);

    sideLayout->addStretch();
    auto *divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color:#344A63;background:#344A63;max-height:1px;");
    sideLayout->addWidget(divider);
    auto *signedIn = new QLabel(QStringLiteral("SIGNED IN AS"));
    signedIn->setStyleSheet("color:#91A0B1;font-size:8pt;font-weight:700;letter-spacing:1px;");
    auto *username = new QLabel(m_user.username);
    username->setStyleSheet("color:#FFFFFF;font-size:11pt;font-weight:600;");
    sideLayout->addWidget(signedIn);
    sideLayout->addWidget(username);
    auto *logoutButton = makeNavButton(QStringLiteral("↪   Sign out"));
    sideLayout->addSpacing(12);
    sideLayout->addWidget(logoutButton);

    auto *mainArea = new QWidget;
    auto *mainLayout = new QVBoxLayout(mainArea);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *topbar = new QFrame;
    topbar->setStyleSheet("background:#FFFFFF;border-bottom:1px solid #E3E7EB;");
    topbar->setFixedHeight(72);
    auto *topLayout = new QHBoxLayout(topbar);
    topLayout->setContentsMargins(28, 10, 28, 10);
    auto *topCaption = new QLabel(QStringLiteral("BANK OPERATIONS  /  MANAGEMENT CONSOLE"));
    topCaption->setStyleSheet("color:#7B8795;font-size:8pt;font-weight:700;letter-spacing:1px;");
    topLayout->addWidget(topCaption);
    topLayout->addStretch();
    auto *userChip = new QLabel(QStringLiteral("  %1   ·   Active session  ").arg(m_user.username));
    userChip->setStyleSheet("background:#F0F2F1;color:#435267;border:1px solid #E0E4E7;border-radius:14px;padding:7px 10px;font-weight:600;");
    topLayout->addWidget(userChip);
    mainLayout->addWidget(topbar);

    m_pages = new QStackedWidget;
    m_dashboardPage = new DashboardPage(m_service, m_user);
    m_clientsPage = new ClientsPage(m_service, m_user);
    m_transactionsPage = new TransactionsPage(m_service, m_user);
    m_usersPage = new UsersPage(m_service, m_user);
    m_pages->addWidget(m_dashboardPage);
    m_pages->addWidget(m_clientsPage);
    m_pages->addWidget(m_transactionsPage);
    m_pages->addWidget(m_usersPage);
    mainLayout->addWidget(m_pages, 1);

    root->addWidget(sidebar);
    root->addWidget(mainArea, 1);
    setCentralWidget(central);

    statusBar()->showMessage(QStringLiteral("Data files: %1").arg(m_service->dataDirectory()));

    m_dashboardNav->setProperty("active", true);
    QObject::connect(m_dashboardNav, &QPushButton::clicked, this, [this] { showPage(0, m_dashboardNav); });
    QObject::connect(m_clientsNav, &QPushButton::clicked, this, [this] { showPage(1, m_clientsNav); });
    QObject::connect(m_transactionsNav, &QPushButton::clicked, this, [this] { showPage(2, m_transactionsNav); });
    QObject::connect(m_usersNav, &QPushButton::clicked, this, [this] { showPage(3, m_usersNav); });
    QObject::connect(logoutButton, &QPushButton::clicked, this, [this] { logout(); });

    m_clientsPage->setOnDataChanged([this] { m_dashboardPage->refresh(); });
    m_transactionsPage->setOnDataChanged([this] { m_dashboardPage->refresh(); });
}

void MainWindow::showPage(int index, QPushButton *activeButton)
{
    m_pages->setCurrentIndex(index);
    const QList<QPushButton *> buttons = {m_dashboardNav, m_clientsNav, m_transactionsNav, m_usersNav};
    for (QPushButton *button : buttons) {
        if (!button) continue;
        button->setProperty("active", button == activeButton);
        button->style()->unpolish(button);
        button->style()->polish(button);
    }
    if (index == 0) m_dashboardPage->refresh();
    if (index == 1) m_clientsPage->refresh();
    if (index == 3) m_usersPage->refresh();
}

void MainWindow::logout()
{
    hide();
    LoginWindow login(m_service, this);
    if (login.exec() == QDialog::Accepted) {
        auto *nextWindow = new MainWindow(m_service, login.authenticatedUser());
        nextWindow->show();
        close();
    } else {
        close();
    }
}
