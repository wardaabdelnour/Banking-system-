#pragma once

#include "BankTypes.h"

#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStringList>

inline QString bankStyleSheet()
{
    return QString::fromUtf8(R"(
        QWidget { font-family: "Segoe UI"; font-size: 10pt; color: #263445; background: #F6F4EE; }
        QLabel, QCheckBox { background: transparent; }
        QMainWindow, QDialog { background: #F6F4EE; }
        QFrame#Sidebar { background: #172B45; border: none; }
        QLabel#BrandTitle { color: #FFFFFF; font-size: 17pt; font-weight: 700; }
        QLabel#BrandCaption { color: #C5CED9; font-size: 8pt; letter-spacing: 1px; }
        QLabel#PageTitle { color: #172B45; font-size: 21pt; font-weight: 700; }
        QLabel#PageSubtitle { color: #758193; font-size: 9pt; }
        QLabel#Muted { color: #7C8794; }
        QLabel#CardTitle { color: #718092; font-size: 9pt; font-weight: 600; }
        QLabel#CardValue { color: #172B45; font-size: 22pt; font-weight: 700; }
        QFrame#Card { background: #FFFFFF; border: 1px solid #E6E9ED; border-radius: 12px; }
        QFrame#SoftPanel { background: #F0F1EF; border: 1px solid #E2E5E8; border-radius: 10px; }
        QPushButton { border: 1px solid #D5DCE3; border-radius: 7px; padding: 8px 14px; background: #FFFFFF; color: #263445; }
        QPushButton:hover { background: #F0F3F6; border-color: #B8C3CE; }
        QPushButton:disabled { color: #A7AFB9; background: #F0F1F2; }
        QPushButton#PrimaryButton { background: #172B45; color: #FFFFFF; border: 1px solid #172B45; font-weight: 600; }
        QPushButton#PrimaryButton:hover { background: #243D5C; }
        QPushButton#GoldButton { background: #B59A63; color: #FFFFFF; border: 1px solid #B59A63; font-weight: 600; }
        QPushButton#GoldButton:hover { background: #A98B50; }
        QPushButton#DangerButton { color: #9A3D3D; border-color: #E4CACA; background: #FFFDFD; }
        QPushButton#NavButton { color: #D7DEE7; background: transparent; border: none; text-align: left; padding: 12px 15px; font-weight: 500; }
        QPushButton#NavButton:hover { background: #263D59; color: #FFFFFF; }
        QPushButton#NavButton[active="true"] { background: #2B4360; color: #FFFFFF; border-left: 3px solid #B59A63; }
        QLineEdit, QComboBox, QDoubleSpinBox, QSpinBox { background: #FFFFFF; border: 1px solid #D6DDE4; border-radius: 7px; padding: 8px 10px; selection-background-color: #B59A63; }
        QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus { border: 1px solid #B59A63; }
        QTableWidget { background: #FFFFFF; alternate-background-color: #F8F9FA; border: 1px solid #E2E6EA; border-radius: 9px; gridline-color: #EDF0F2; selection-background-color: #E7ECF1; selection-color: #172B45; }
        QHeaderView::section { background: #EEF1F4; color: #586678; border: none; border-bottom: 1px solid #DDE3E8; padding: 10px 8px; font-weight: 600; }
        QTabWidget::pane { border: 1px solid #E2E6EA; border-radius: 9px; background: #FFFFFF; top: -1px; }
        QTabBar::tab { background: #ECEFEE; padding: 10px 18px; margin-right: 4px; border-top-left-radius: 7px; border-top-right-radius: 7px; color: #536174; }
        QTabBar::tab:selected { background: #FFFFFF; color: #172B45; font-weight: 700; border-bottom: 2px solid #B59A63; }
        QCheckBox { spacing: 8px; }
        QCheckBox::indicator { width: 16px; height: 16px; }
        QStatusBar { background: #FFFFFF; color: #7C8794; border-top: 1px solid #E5E8EB; }
        QDialogButtonBox QPushButton { min-width: 85px; }
    )");
}

inline void showError(QWidget *parent, const QString &message)
{
    QMessageBox::warning(parent, QStringLiteral("Banking System"), message);
}

inline void showInfo(QWidget *parent, const QString &message)
{
    QMessageBox::information(parent, QStringLiteral("Banking System"), message);
}

inline QFrame *makeCard(QWidget *parent = nullptr)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName("Card");
    return frame;
}

inline QLabel *makePageTitle(const QString &title, const QString &subtitle, QVBoxLayout *layout)
{
    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName("PageTitle");
    auto *subtitleLabel = new QLabel(subtitle);
    subtitleLabel->setObjectName("PageSubtitle");
    layout->addWidget(titleLabel);
    layout->addWidget(subtitleLabel);
    layout->addSpacing(12);
    return titleLabel;
}

inline void configureTable(QTableWidget *table)
{
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->setWordWrap(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->setMinimumHeight(260);
}

inline QString permissionSummary(int permissions)
{
    if (hasFullAccess(permissions))
        return QStringLiteral("Full access");

    QStringList items;
    if (permissions & BankPermissions::ListClients) items << "List clients";
    if (permissions & BankPermissions::AddClient) items << "Add clients";
    if (permissions & BankPermissions::DeleteClient) items << "Delete clients";
    if (permissions & BankPermissions::UpdateClient) items << "Update clients";
    if (permissions & BankPermissions::FindClient) items << "Find clients";
    if (permissions & BankPermissions::Transactions) items << "Transactions";
    if (permissions & BankPermissions::ManageUsers) items << "Manage users";
    if (items.isEmpty())
        return QStringLiteral("No access");
    return items.join(QStringLiteral(" · "));
}
