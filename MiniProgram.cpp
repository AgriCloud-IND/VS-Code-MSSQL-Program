#include <iostream>
#include <Windows.h>
#include "sqltypes.h"
#include "sql.h"
#include "sqlext.h"
#include <fstream>  // File Handling
#include <cstdlib>  // Open Default Browser
#include <codecvt>  // For std::codecvt_utf8
#include <locale>   // For std::wstring_convert
#include <vector>

using namespace std;

class Database
{
private:
    static Database* instance; // Singleton instance
    SQLHANDLE sqlenvhandle;
    SQLHANDLE sqlconnectionhandle;
    SQLHANDLE sqlstatementhandle;
    SQLRETURN retcode_conn;
    SQLRETURN retcode_stmt;

    Database()
    {
        retcode_conn = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlenvhandle);
        retcode_conn = SQLSetEnvAttr(sqlenvhandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

        // Initialize connection handle
        retcode_conn = SQLAllocHandle(SQL_HANDLE_DBC, sqlenvhandle, &sqlconnectionhandle);

        // Connection string with Windows Authentication
        SQLCHAR* connection_string = (SQLCHAR*)"Driver={SQL Server};Server=MMC\\SQLEXPRESS;Database=MMC;Trusted_Connection=yes;";

        // Connect to the database
        retcode_conn = SQLDriverConnect(sqlconnectionhandle, NULL, connection_string, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

        if (retcode_conn == SQL_SUCCESS || retcode_conn == SQL_SUCCESS_WITH_INFO)
        {
            cout << "Connected to the database using Windows Authentication." << endl;
        }
        else
        {
            // Handle connection error
            cout << "Failed to connect to the database." << endl;
        }
    }

public:
    static Database* getInstance()
    {
        if (!instance)
        {
            instance = new Database();
        }
        return instance;
    }

    ~Database()
    {
        // Cleanup connection handle
        SQLDisconnect(sqlconnectionhandle);
        SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
        SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);
    }

    int Datbase_Insert(const string& sqlStmtInsert)
    {
        // Initialize statement handle
        retcode_stmt = SQLAllocHandle(SQL_HANDLE_STMT, sqlconnectionhandle, &sqlstatementhandle);
        if (retcode_stmt == SQL_SUCCESS || retcode_stmt == SQL_SUCCESS_WITH_INFO)
        {
            // Construct the SQL statement using wstring
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

            // Convert std::string to std::wstring
            wstring insertStatement = converter.from_bytes(sqlStmtInsert);

            // Convert wstring to SQLWCHAR* and execute
            const SQLWCHAR* sqlInsert = insertStatement.c_str();

            retcode_stmt = SQLExecDirectW(sqlstatementhandle, (SQLWCHAR*)sqlInsert, SQL_NTS);

            if (retcode_stmt == SQL_SUCCESS || retcode_stmt == SQL_SUCCESS_WITH_INFO)
            {
                cout << "Data inserted successfully." << endl;
            }
            else
            {
                SQLWCHAR sqlstate[6];
                SQLWCHAR message[SQL_MAX_MESSAGE_LENGTH];
                SQLINTEGER nativeError;
                SQLSMALLINT messageLength;

                SQLGetDiagRecW(SQL_HANDLE_STMT, sqlstatementhandle, 1, sqlstate, &nativeError, message, sizeof(message), &messageLength);
                cout << "SQL Error: " << sqlstate << " - " << message << endl;
            }
        }
        else
        {
            cout << "Datbase_Insert: Error in Initializing statement handle" << endl;
        }
        return retcode_stmt;
    }
};

Database* Database::instance = nullptr;

class User
{
public:
    int Id;
    std::string Name;
    std::string Email;

    User(){};
    ~User(){};

    void Create_User()
    {
        std::string sqlStmtInsert = "Insert into Users(Name, Email) Values('" + Name + "','" + Email + "');";
        Database* DB = Database::getInstance();
        std::cout << DB->Datbase_Insert(sqlStmtInsert);
    }
};

int main()
{
    std::cout << "Welcome to MiniProject" << std::endl;
    std::cout << "Here are the Options" << std::endl;
    std::cout << "1. Creating User" << std::endl;

    int Option;
    std::cout << "Enter your Option: ";
    std::cin >> Option;

    switch (Option)
    {
    case 1:
        do
        {
            User U;
            std::cout << "Enter Name: ";
            std::cin >> U.Name;
            std::cout << "Enter Email: ";
            std::cin >> U.Email;
            U.Create_User();
            std::cout << "Would you like to Add more Users? Press 1 for yes, 0 for no: ";
            std::cin >> Option;
        } while (Option);
        break;
    }

    return 0;
}
