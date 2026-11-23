#include "LoginWindow.h"
#include "UiHelpers.h"

#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginWindow::LoginWindow(BankService *service, QWidget *parent)
    : QDialog(parent), m_service(service),
      m_usernameEdit(nullptr), m_passwordEdit(nullptr)
{
    setWindowTitle(QStringLiteral("Sign in | Meridian Bank"));
    setModal(true);
    setFixedSize(470, 570);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(34, 30, 34, 30);
    outer->setSpacing(0);

    auto *brand = new QLabel(QStringLiteral("MERIDIAN"));
    brand->setStyleSheet("color:#172B45;font-size:24pt;font-weight:800;letter-spacing:3px;");
    brand->setAlignment(Qt::AlignCenter);
    outer->addWidget(brand);

    auto *caption = new QLabel(QStringLiteral("PRIVATE BANKING  ·  SECURE ACCESS"));
    caption->setAlignment(Qt::AlignCenter);
    caption->setStyleSheet("color:#9A8459;font-size:8pt;font-weight:700;letter-spacing:2px;");
    outer->addWidget(caption);
    outer->addSpacing(28);

    auto *panel = makeCard(this);
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(28, 28, 28, 28);
    panelLayout->setSpacing(13);

    auto *heading = new QLabel(QStringLiteral("Welcome back"));
    heading->setStyleSheet("color:#172B45;font-size:19pt;font-weight:700;");
    panelLayout->addWidget(heading);
    auto *subheading = new QLabel(QStringLiteral("Sign in to access your banking workspace."));
    subheading->setObjectName("Muted");
    panelLayout->addWidget(subheading);
    panelLayout->addSpacing(8);

    auto *userLabel = new QLabel(QStringLiteral("USERNAME"));
    userLabel->setStyleSheet("color:#59687A;font-size:8pt;font-weight:700;letter-spacing:1px;");
    panelLayout->addWidget(userLabel);
    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setPlaceholderText(QStringLiteral("Enter your username"));
    m_usernameEdit->setMinimumHeight(42);
    panelLayout->addWidget(m_usernameEdit);

    auto *passwordLabel = new QLabel(QStringLiteral("PASSWORD"));
    passwordLabel->setStyleSheet("color:#59687A;font-size:8pt;font-weight:700;letter-spacing:1px;");
    panelLayout->addWidget(passwordLabel);
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setPlaceholderText(QStringLiteral("Enter your password"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(42);
    panelLayout->addWidget(m_passwordEdit);

    auto *showPassword = new QCheckBox(QStringLiteral("Show password"));
    panelLayout->addWidget(showPassword);
    QObject::connect(showPassword, &QCheckBox::toggled, this, [this](bool checked) {
        m_passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    auto *loginButton = new QPushButton(QStringLiteral("Sign in"));
    loginButton->setObjectName("PrimaryButton");
    loginButton->setMinimumHeight(44);
    panelLayout->addSpacing(8);
    panelLayout->addWidget(loginButton);
    outer->addWidget(panel);
    outer->addStretch();

    auto *footer = new QLabel(QStringLiteral("A calm, secure space for everyday banking."));
    footer->setAlignment(Qt::AlignCenter);
    footer->setObjectName("Muted");
    outer->addWidget(footer);

    QObject::connect(loginButton, &QPushButton::clicked, this, [this]() {
        const QString username = m_usernameEdit->text().trimmed();
        const QString password = m_passwordEdit->text();
        if (username.isEmpty() || password.isEmpty()) {
            showError(this, QStringLiteral("Enter both username and password."));
            return;
        }

        QString error;
        UserRecord signedIn;
        if (!m_service->authenticate(username, password, &signedIn, &error)) {
            showError(this, error);
            m_passwordEdit->clear();
            m_passwordEdit->setFocus();
            return;
        }
        m_authenticatedUser = signedIn;
        accept();
    });

    QObject::connect(m_passwordEdit, &QLineEdit::returnPressed, loginButton, &QPushButton::click);
    m_usernameEdit->setFocus();
}

UserRecord LoginWindow::authenticatedUser() const
{
    return m_authenticatedUser;
}
