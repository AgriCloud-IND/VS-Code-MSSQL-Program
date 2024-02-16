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
#include <regex>  // For Regular Expression Validation (Utility Class)

class CUtility
{
    public :
        // Function to validate a name
        static bool isValidName(const std::string& name) {
            // Name should only contain alphabets and spaces
            return std::regex_match(name, std::regex("^[a-zA-Z ]+$"));
        }

        // Function to validate an email address
        static bool isValidEmail(const std::string& email) {
            // Regular expression for email validation
            return std::regex_match(email, std::regex("^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$"));
        }
        // Function to validate a phone number
        static bool isValidPhone(const std::string& phone) {
            // Regular expression for phone number validation
            return std::regex_match(phone, std::regex("^\\d{10}$"));
        }



};

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

/*
CREATE TABLE [dbo].[Users](
	[UserID] [int] IDENTITY(1,1) NOT NULL,
	[Name] [varchar](50) NULL,
	[Email] [varchar](50) NULL,
 CONSTRAINT [PK_Users] PRIMARY KEY CLUSTERED 
(
	[UserID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]

*/

public:
    int Id;
    std::string Name;
    std::string Email;

    User(){};
    ~User(){};

    void Create_User()
    {
        std::cout<<"Debug Create_User" <<std::endl;  
        do{
                std::cout<<"Enter Name:";
                std::cin>>Name;
        }while(!CUtility::isValidName(Name));
        
        do{
                std::cout<<"Enter Email:";
                std::cin>>Email;
        
        }while(!CUtility::isValidEmail(Email));

        std::string sqlStmtInsert = "Insert into Users(Name, Email) Values('" + Name + "','" + Email + "');";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtInsert<<std::endl;
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
        std::cout<<sqlStmtUpdate<<std::endl;
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
        std::cout<<sqlStmtDelete<<std::endl;
        std::cout << DB->Datbase_Create_Update_Delete(sqlStmtDelete);
    }
    void DisplayAll()
    {
        std::string sqlStmtSelete = "Select UserID, Name, Email from Users;";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtSelete<<std::endl;
        DB->Database_DisplayAll(sqlStmtSelete);
            
    }        
};

class Customers
{

/* 
CREATE TABLE [dbo].[customers](
	[customer_id] [int] IDENTITY(1,1) NOT NULL,
	[customer_name] [nvarchar](50) NULL,
	[customer_email] [nvarchar](50) NULL,
	[customer_phone] [nvarchar](50) NULL,
	[customer_address] [nvarchar](250) NULL
) ON [PRIMARY]
GO
*/

public:
    int customer_id;
    std::string customer_name;
    std::string customer_email;
    std::string customer_phone;
    std::string customer_address;

    Customers(){};
    ~Customers(){};

    void Create_Customers()
    {

        std::string sName, sEmail,sPhone,sAddress;
        std::cout<<"Enter Name:";
        std::cin>>customer_name;
        std::cout<<"Enter Email:";
        std::cin>>customer_email;
        do{
            std::cout<<"Enter sPhone:";
            std::cin>>customer_phone;
        }while(!CUtility::isValidPhone(customer_phone));

        std::cout<<"Enter sAddress:";
        std::cin>>customer_address;
        
        std::string sqlStmtInsert = "Insert into Customers(customer_name, customer_email,customer_phone,customer_address) Values('" + customer_name + "','" + customer_email + "','"+ customer_phone+"','"+ customer_address +"');";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtInsert<<std::endl;
        std::cout << DB->Datbase_Create_Update_Delete(sqlStmtInsert);
    }

    
    void Update_Customers()
    {
        int iId;
        std::cout<<"Select the ID for Update"<<endl;
        DisplayAll();
        cin>>iId;
        std::string sName, sEmail,sPhone,sAddress;
        std::cout<<"Enter Name:";
        std::cin>>sName;
        std::cout<<"Enter Email:";
        std::cin>>sEmail;
        std::cout<<"Enter sPhone:";
        std::cin>>sPhone;
        std::cout<<"Enter sAddress:";
        std::cin>>sAddress;
        
        std::string sqlStmtUpdate = "Update Customers SET customer_name='" + sName +"',customer_email='" + sEmail + "', customer_phone='" + sPhone + "', customer_address='" + sAddress +"' where customer_id=" + std::to_string(iId);
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtUpdate<<std::endl;
        std::cout << DB->Datbase_Create_Update_Delete(sqlStmtUpdate);
    }

     void Delete_Customers()
    {
        int iId;
        std::cout<<"Select the ID for Delete"<<endl;
        DisplayAll();
        cin>>iId;
        std::string sqlStmtDelete = "Delete From Customers where customer_id=" + std::to_string(iId);
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtDelete<<std::endl;
        std::cout << DB->Datbase_Create_Update_Delete(sqlStmtDelete);
    }
    void DisplayAll()
    {
        std::string sqlStmtSelete = "Select * from Customers;";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtSelete<<std::endl;
        DB->Database_DisplayAll(sqlStmtSelete);
            
    } 
    

};

/*
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
*/

int main()
{
    int Option;
    do
    {
        std::cout << "Welcome to MiniProject" << std::endl;
        std::cout << "1. User Management" << std::endl;
        std::cout << "2. Customer Management" << std::endl;
        
        std::cout << "Enter your Option: ";
        std::cin >> Option;

        switch (Option)
        {
            case 1: //User Management 
            {
                    std::cout << "Following are the Options for User Management" << std::endl;
                    std::cout << "1. Create " << std::endl;
                    std::cout << "2. Update " << std::endl;
                    std::cout << "3. Delete " << std::endl;
                    std::cout << "4. View " << std::endl;
                    std::cout << "Enter your Option: ";
                    std::cin >> Option;
                    User objUser;
                    switch(Option)
                    {
                        case 1: //Add New Record
                            do{
                                objUser.Create_User();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no: "<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;
                        case 2: //Update Record
                            do{
                                objUser.Update_User();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no: "<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;
                        case 3://Delete Record
                            do{
                                objUser.Delete_User();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no: "<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;
                        case 4://View Record
                            do{
                                objUser.DisplayAll();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no: "<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;

                    } //Close of Operations for Users 
            } //Cloase Case for user Management 
            break; // Break for User Management  
            case 2: //User Management 
            {
                    std::cout << "Following are the Options for Customer Management" << std::endl;
                    std::cout << "1. Create " << std::endl;
                    std::cout << "2. Update " << std::endl;
                    std::cout << "3. Delete " << std::endl;
                    std::cout << "4. View " << std::endl;
                    std::cout << "Enter your Option: ";
                    std::cin >> Option;
                    Customers objCust;
                    switch(Option)
                    {
                        case 1: //Add New Record
                            do{
                                objCust.Create_Customers();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no:"<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;
                        case 2: //Update Record
                            do{
                                objCust.Update_Customers();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no: "<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;
                        case 3://Delete Record
                            do{
                                objCust.Delete_Customers();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no: "<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;
                        case 4://View Record
                            do{
                                objCust.DisplayAll();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no: "<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;

                    } //Close of Operations for Customer 
            } //Cloase Case for Customer Management 
            break; // Break for Customer Management 
        }
            std::cout << "Would you like to perform Other Management? Press 1 for yes, 0 for no: "<<std::endl;
            std::cin >> Option;
        } while (Option); 
    return 0;
}
