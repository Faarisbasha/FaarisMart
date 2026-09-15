

#include <iostream>
#include <string>
#include <conio.h>   // For password hiding
using namespace std;

string getPassword() {
    string password = "";
    char ch;

    while ((ch = _getch()) != 13) { // Enter key
        if (ch == 8) { // Backspace
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
        } else {
            password += ch;
            cout << "*";
        }
    }
    cout << endl;
    return password;
}

int main() {
    string correctUsername = "admin";
    string correctPassword = "Shop@123";

    string username, password;
    int attempts = 3;

    cout << "=================================\n";
    cout << "      FAARISMART LOGIN PAGE\n";
    cout << "=================================\n\n";

    while (attempts > 0) {
        cout << "Username: ";
        cin >> username;

        cout << "Password: ";
        password = getPassword();

        // Validation
        if (username.empty()) {
            cout << "Username cannot be empty.\n\n";
            continue;
        }

        if (password.length() < 6) {
            cout << "Password must be at least 6 characters.\n\n";
            continue;
        }

        if (username == correctUsername && password == correctPassword) {
            cout << "\nLogin Successful!\n";
            cout << "Welcome to FaarisMart.\n";
            return 0;
        } else {
            attempts--;
            cout << "\nInvalid Username or Password.\n";

            if (attempts > 0)
                cout << "Attempts left: " << attempts << "\n\n";
        }
    }

    cout << "\nAccount Locked! Too many failed attempts.\n";
    return 0;
}