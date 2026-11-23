#include "LoginWindow.h"
#include "MainWindow.h"
#include "UiHelpers.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Meridian Bank"));
    app.setOrganizationName(QStringLiteral("Meridian"));
    app.setStyleSheet(bankStyleSheet());

    BankService service;
    LoginWindow login(&service);
    if (login.exec() != QDialog::Accepted)
        return 0;

    auto *window = new MainWindow(&service, login.authenticatedUser());
    window->show();
    return app.exec();
}
