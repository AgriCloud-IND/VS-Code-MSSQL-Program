#include <iostream>
#include <Windows.h>
#include "sqltypes.h"
#include "sql.h"
#include "sqlext.h"
#include <fstream>  // File Handling
#include <cstdlib>  // Open Default Browser
#include <codecvt>  // For std::codecvt_utf8
#include <locale>   // For std::wstring_convert

using namespace std;

int main() {
    SQLHANDLE sqlenvhandle;
    SQLHANDLE sqlconnectionhandle;
    SQLHANDLE sqlstatementhandle;
    SQLRETURN retcode_conn;
    SQLRETURN retcode_stmt;

    // Initialize environment handle
    retcode_conn = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlenvhandle);
    retcode_conn = SQLSetEnvAttr(sqlenvhandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    // Initialize connection handle
    retcode_conn = SQLAllocHandle(SQL_HANDLE_DBC, sqlenvhandle, &sqlconnectionhandle);

    // Connection string with Windows Authentication
    SQLCHAR* connection_string = (SQLCHAR*)"Driver={SQL Server};Server=MMC\\SQLEXPRESS;Database=MMC;Trusted_Connection=yes;";

    // Connect to the database
    retcode_conn = SQLDriverConnect(sqlconnectionhandle, NULL, connection_string, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

    if (retcode_conn == SQL_SUCCESS || retcode_conn == SQL_SUCCESS_WITH_INFO) {
        cout << "Connected to the database using Windows Authentication." << endl;

        // ... [Rest of the database operations]

        char id[50];
        char Name[50];
        char Email[50];

        // Initialize statement handle
        retcode_stmt = SQLAllocHandle(SQL_HANDLE_STMT, sqlconnectionhandle, &sqlstatementhandle);

        if (retcode_stmt == SQL_SUCCESS || retcode_stmt == SQL_SUCCESS_WITH_INFO) {
            char operation = '+';
            cout << "Enter an operation: ";
            cin >> operation;

            switch (operation) {
                case '+': {
                    cout << "Enter Name \n";
                    cin >> Name;

                    cout << "Enter Email \n";
                    cin >> Email;

                    // Convert char arrays to wstring
                    wstring wName = wstring(Name, Name + strlen(Name));
                    wstring wEmail = wstring(Email, Email + strlen(Email));

                    // Construct the SQL statement using wstring
                    wstring insertStatement = L"INSERT INTO Users (Name, Email) VALUES ('" + wName + L"','" + wEmail + L"')";

                    // Convert wstring to SQLWCHAR* and execute
                    const SQLWCHAR* sqlInsert = insertStatement.c_str();

                    retcode_stmt = SQLExecDirectW(sqlstatementhandle, (SQLWCHAR*)sqlInsert, SQL_NTS);

                    if (retcode_stmt == SQL_SUCCESS || retcode_stmt == SQL_SUCCESS_WITH_INFO) {
                        cout << "Data inserted successfully." << endl;
                    } else {
                        SQLWCHAR sqlstate[6];
                        SQLWCHAR message[SQL_MAX_MESSAGE_LENGTH];
                        SQLINTEGER nativeError;
                        SQLSMALLINT messageLength;

                        SQLGetDiagRecW(SQL_HANDLE_STMT, sqlstatementhandle, 1, sqlstate, &nativeError, message, sizeof(message), &messageLength);

                        cout << "SQL Error: " << sqlstate << " - " << message << endl;
                    }
                    break;
                }
                default: {
                    std::ofstream outputFile("Output.html");

                    SQLWCHAR* selectStatement = (SQLWCHAR*)L"SELECT UserID, Name, Email FROM Users";

                    retcode_stmt = SQLExecDirectW(sqlstatementhandle, selectStatement, SQL_NTS);

                    // Create and open a file
                    if (outputFile.is_open()) {
                        // Write content to the file
                        cout << "File created successfully." << endl;
                    } else {
                        cerr << "Error opening the file." << endl;
                    }

                    wcout << "<table border='1'>\n";
                    wcout << "<tr><th>UserID</th><th>Name</th><th>Email</th></tr>\n";

                    outputFile << "<table border='1'>\n";
                    outputFile << "<tr><th>UserID</th><th>Name</th><th>Email</th></tr>\n";

                    if (retcode_stmt == SQL_SUCCESS || retcode_stmt == SQL_SUCCESS_WITH_INFO) {
                        SQLWCHAR SelectUserID[50];
                        SQLWCHAR Selectname[50];
                        SQLWCHAR Selectemail[50];

                        while (SQLFetch(sqlstatementhandle) == SQL_SUCCESS) {
                            SQLGetData(sqlstatementhandle, 1, SQL_C_WCHAR, SelectUserID, sizeof(SelectUserID), NULL);
                            SQLGetData(sqlstatementhandle, 2, SQL_C_WCHAR, Selectname, sizeof(Selectname), NULL);
                            SQLGetData(sqlstatementhandle, 3, SQL_C_WCHAR, Selectemail, sizeof(Selectemail), NULL);

                            wcout << "<tr><td>" << SelectUserID << "</td><td>" << Selectname << "</td><td>" << Selectemail << "</td></tr>\n";
                            
                            // Convert SQLWCHAR to UTF-8
                            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                            outputFile << "<tr><td>" << converter.to_bytes(SelectUserID) << "</td><td>" << converter.to_bytes(Selectname) << "</td><td>" << converter.to_bytes(Selectemail) << "</td></tr>\n";
                        }
                    } else {
                        wcout << "<tr><td colspan='2'>Error executing SELECT statement.</td></tr>\n";
                        outputFile << "<tr><td colspan='2'>Error executing SELECT statement.</td></tr>\n";
                    }

                    wcout << "</table>";
                    outputFile << "</table>";
                    outputFile.close();
                    cout << "File created successfully." << endl;

                    // Opening a file in Browser -
                    {
                        const char* htmlFile = "Output.html";

                        // Form the command to open the default web browser with the given HTML file
                        const char* command = "start %s";
                        char fullCommand[255];
                        sprintf(fullCommand, command, htmlFile);

                        // Execute the command
                        system(fullCommand);
                    }

                    break;
                }
            }
        }

        // Cleanup statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
    } else {
        // Handle connection error
        cout << "Failed to connect to the database." << endl;
    }

    // Cleanup connection handle
    SQLDisconnect(sqlconnectionhandle);
    SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);

    return 0;
}
