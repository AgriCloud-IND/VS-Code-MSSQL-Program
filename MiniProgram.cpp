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
#include <string>
#include <cstdlib> // for atoi function

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

    int Datbase_Create_Update_Delete(const string& sqlStmtInsert)
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
                cout << "Record Save Successfully." << endl;
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
            cout << "Datbase_Create_Update_Delete: Error in Initializing statement handle" << endl;
        }
        return retcode_stmt;
    }
    void Database_DisplayAll(const string& sqlSelect)
    {
      
       // Initialize statement handle
        std::cout<<sqlSelect<<endl;
        retcode_stmt = SQLAllocHandle(SQL_HANDLE_STMT, sqlconnectionhandle, &sqlstatementhandle);
        if (retcode_stmt == SQL_SUCCESS || retcode_stmt == SQL_SUCCESS_WITH_INFO)
        {
       
            // Construct the SQL statement using wstring
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

            // Convert std::string to std::wstring
            wstring wsqlStatement = converter.from_bytes(sqlSelect);

            // Convert wstring to SQLWCHAR* and execute
            const SQLWCHAR* selectStatement = wsqlStatement.c_str();

            retcode_stmt = SQLExecDirectW(sqlstatementhandle, const_cast<SQLWCHAR*>(selectStatement), SQL_NTS);


            wcout << "<table border='1'>\n";
            
            if (retcode_stmt == SQL_SUCCESS || retcode_stmt == SQL_SUCCESS_WITH_INFO) {
                SQLWCHAR SelectUserID[50];
                SQLWCHAR Selectname[50];
                SQLWCHAR Selectemail[50];

                wcout << "<tr><th>UserID</th><th>Name</th><th>Email</th></tr>\n";

                while (SQLFetch(sqlstatementhandle) == SQL_SUCCESS) {
                    SQLGetData(sqlstatementhandle, 1, SQL_C_WCHAR, SelectUserID, sizeof(SelectUserID), NULL);
                    SQLGetData(sqlstatementhandle, 2, SQL_C_WCHAR, Selectname, sizeof(Selectname), NULL);
                    SQLGetData(sqlstatementhandle, 3, SQL_C_WCHAR, Selectemail, sizeof(Selectemail), NULL);

                    wcout << "<tr><td>" << SelectUserID << "</td><td>" << Selectname << "</td><td>" << Selectemail << "</td></tr>\n";
                }
            } else {
                wcout << "<tr><td colspan='2'>Error executing SELECT statement.</td></tr>\n";

                SQLWCHAR sqlstate[6];
                SQLWCHAR message[SQL_MAX_MESSAGE_LENGTH];
                SQLINTEGER nativeError;
                SQLSMALLINT messageLength;

                // Convert SQLWCHAR to UTF-8
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                            

                SQLGetDiagRecW(SQL_HANDLE_STMT, sqlstatementhandle, 1, sqlstate, &nativeError, message, sizeof(message), &messageLength);
                cout << "SQL Error: " << converter.to_bytes(sqlstate) << " - " << converter.to_bytes(message) << endl;

            }
            wcout << "</table>";
        }
        else
        {
            cout << "Database_DisplayAll: Error in Initializing statement handle" << endl;
        }
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
        std::cout << DB->Datbase_Create_Update_Delete(sqlStmtInsert);
    }
    void Update_User()
    {
        int iId;
        std::cout<<"Select the ID for Update"<<endl;
        DisplayAll();
        cin>>iId;
        std::string sName, sEmail;
        std::cout<<"Enter Name:";
        std::cin>>sName;
        std::cout<<"Enter Email:";
        std::cin>>sEmail;
        std::string sqlStmtUpdate = "Update Users SET Name='" + sName +"',Email='" + sEmail + "' where UserID=" + std::to_string(iId);
        Database* DB = Database::getInstance();
        std::cout << DB->Datbase_Create_Update_Delete(sqlStmtUpdate);
    }
     void Delete_User()
    {
        int iId;
        std::cout<<"Select the ID for Update"<<endl;
        DisplayAll();
        cin>>iId;
        std::string sqlStmtDelete = "Delete From Users where UserID=" + std::to_string(iId);
        Database* DB = Database::getInstance();
        std::cout << DB->Datbase_Create_Update_Delete(sqlStmtDelete);
    }
    void DisplayAll()
    {
        std::string sqlStmtSelete = "Select UserID, Name, Email from Users;";
        Database* DB = Database::getInstance();
        DB->Database_DisplayAll(sqlStmtSelete);
            
    }        
};

class Customer 
{
public:
    std::string CustomerID;
    std::string CompanyName;
    std::string ContactName;
    std::string ContactTitle;
    std::string Address;
    std::string City;
    std::string Region;
    std::string PostalCode;
    std::string Country;
    std::string Phone;
    std::string Fax;
    static std::vector<Customer*> vecCustomers;

    void static createCustomers()
    {
        for (const auto objCustomer : vecCustomers) {
           Create_Customer(objCustomer);
        }
    }

    int static Create_Customer(const Customer* Obj)
    {
            std::string sqlStmtInsert = "INSERT INTO [dbo].[Customers] ([CustomerId],[CompanyName],[ContactName],[ContactTitle],[Address],[City],[Region],[PostalCode],[Country],[Phone],[Fax]) VALUES ('" + Obj->CustomerID + "','" + Obj->CompanyName + "','" + Obj->ContactName + "','" + Obj->ContactTitle + "','" + Obj->Address + "','" + Obj->City + "','" + Obj->Region + "','" + Obj->PostalCode + "','" + Obj->Country + "','" + Obj->Phone + "','" + Obj->Fax + "')";
            Database* DB = Database::getInstance();
     
            return DB->Datbase_Create_Update_Delete(sqlStmtInsert);
    }


};


std::vector<Customer*> Customer::vecCustomers;

int main()
{
    std::cout << "Welcome to MiniProject" << std::endl;
    std::cout << "Here are the Options" << std::endl;
    std::cout << "1. Creating User" << std::endl;
    std::cout << "2. Update User" << std::endl;
    std::cout << "3. Delete Users" << std::endl;
    std::cout << "4. Display Users" << std::endl;
    std::cout << "5. Creating Bulk Customer" << std::endl;
    
    int Option;
    std::cout << "Enter your Option: ";
    std::cin >> Option;

    switch (Option)
    {
    case 1: //Create Users
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
    case 2: //Update Users
            do
            {
                User U;
                U.Update_User();
                std::cout << "Would you like to Udpate more Users? Press 1 for yes, 0 for no: ";
                std::cin >> Option;
            } while (Option);
            break;
    case 3: //Delete Users
            do
            {
                User U;
                U.Delete_User();
                std::cout << "Would you like to Delete more Users? Press 1 for yes, 0 for no: ";
                std::cin >> Option;
            } while (Option);
            break;
    
    case 4: //Display Users
           { 
            User U;
            U.DisplayAll();
           }
            break;
    case 5: //Creating Bulk Customers 
        do
        {
            Customer* objCustomer=new Customer();

            std::cout<<"Enter CustomerID:"<<std::endl;
            std::cin>>objCustomer->CustomerID;

            std::cout<<"Enter CompanyName "<<std::endl;
            std::cin>>objCustomer->CompanyName;

            std::cout<<"Enter ContactName:"<<std::endl;
            std::cin>>objCustomer->ContactName;
            
            std::cout<<"Enter ContactTitle:"<<std::endl;
            std::cin>>objCustomer->ContactTitle;

            std::cout<<"Enter Address:"<<std::endl;
            std::cin>>objCustomer->Address;
            
            std::cout<<"Enter City:"<<std::endl;
            std::cin>>objCustomer->City;

            std::cout<<"Enter Region:"<<std::endl;
            std::cin>>objCustomer->Region;
            
            std::cout<<"Enter PostalCode:"<<std::endl;
            std::cin>>objCustomer->PostalCode;

            std::cout<<"Enter Country:"<<std::endl;
            std::cin>>objCustomer->Country;

            std::cout<<"Enter Phone:"<<std::endl;
            std::cin>>objCustomer->Phone;
            
            std::cout<<"Enter Fax:"<<std::endl;
            std::cin>>objCustomer->Fax;

            Customer::vecCustomers.push_back(objCustomer);             
            std::cout << "Would you like to Add more Customers? Press 1 for yes, 0 for no: ";
            std::cin >> Option;
        } while (Option);
        Customer::createCustomers();       
        break;

    }

    return 0;
}
