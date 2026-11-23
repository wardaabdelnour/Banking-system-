# Meridian Bank — Qt 6 Desktop Banking Application

A desktop banking management project built with **C++17 and Qt 6 Widgets**. It provides a graphical interface for account management, transactions, user administration, and bitwise access permissions.

> **Educational project only.** This is not production banking software and must not be used with real customer records, real account credentials, or real money.

## Features

- Login screen backed by a local users file
- Dashboard with client and balance summaries, subject to permissions
- Client management: list, search, add, update, and delete
- Transactions: deposits, withdrawals, and total balance calculation
- User management: add, update, delete, and view permission assignments
- Bitwise role permissions enforced in the service layer
- Local text-file persistence using the original `#//#` record separator
- Atomic file replacement through Qt `QSaveFile`
- Separate UI, service, and data-model files

## Tech stack

- C++17
- Qt 6 Widgets (Qt 6.2 or newer)
- CMake (recommended) or qmake
- Qt Creator

## Screenshots

Add screenshots of the running application here before sharing the repository. Recommended captures:

1. Login screen
2. Dashboard
3. Client management
4. Transactions
5. User management and permissions

Store the images under `docs/screenshots/` and link them from this section.

## Project structure

- `main.cpp` — application entry point
- `LoginWindow.*` — authentication UI
- `MainWindow.*` — navigation and dashboard shell
- `Pages.*` — dashboard, client, transaction, and user-management pages
- `BankService.*` — data access, validation, persistence, and permission checks
- `BankTypes.h` — client/user records and permission constants
- `UiHelpers.h` — shared styling and UI helpers
- `original_backend/` — preserved console backend source; it is not compiled into the Qt GUI target
- `sample_data/` — fictional demo data templates; copy these to the project root to run the demo

## Run with demo data

The application expects `Bank.txt` and `users.txt` in its data directory. These local runtime files are intentionally excluded from Git by `.gitignore`.

### Windows PowerShell

From the project root, run:

```powershell
Copy-Item sample_data/Bank.txt.example Bank.txt
Copy-Item sample_data/users.txt.example users.txt
```

### Demo accounts

| Role | Username | Password | Permissions |
|---|---|---|---|
| Administrator | `demo_admin` | `DemoOnly123!` | Full access |
| Limited user | `demo_viewer` | `ViewOnly123!` | List and find clients only |

These credentials are public demo credentials, not secure secrets. Do not reuse them outside this sample project.

## Build with Qt Creator

1. Install Qt 6 Widgets and a matching compiler kit (for example, Qt 6.12 MinGW 64-bit).
2. Open `CMakeLists.txt` in Qt Creator.
3. Select the matching Qt 6 kit.
4. Configure the project.
5. Set the Run working directory to the project root (the directory containing the generated `Bank.txt` and `users.txt`).
6. Build and run.

### Build from a terminal

```bash
cmake -S . -B build
cmake --build build --config Release
```

The Qt installation must be discoverable by CMake, for example by configuring `CMAKE_PREFIX_PATH` to the Qt installation directory.

## Data format

The app keeps the project's simple text-file format:

- Client: `accountNumber#//#pinCode#//#name#//#balance`
- User: `username#//#password#//#permissions`

The permission flags are `1, 2, 4, 8, 16, 32, 64`; `-1` represents full access.

## Important limitations

- User passwords and client PINs are stored as plain text to remain compatible with the original learning-project file format. The app does not implement password hashing or file encryption.
- The data is stored locally in text files, not in a database.
- The project updates current balances but does not maintain an auditable transaction history.
- This design is suitable for learning and portfolio demonstration only, not real banking or production deployment.
- Use fictional demo data only. Never commit your personal `Bank.txt` or `users.txt` files.

## Original console backend

The original console source is preserved under `original_backend/`. The Qt GUI uses a separate `BankService` because the console functions rely on `cin`, `cout`, and console navigation, which should not be called directly from Qt button handlers.

## License

No open-source license is included yet. Add a license only after deciding how you want others to use, modify, and redistribute this project.
