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
#include <ctime>
#include <map>
#include <shellapi.h>  // For Printer

using namespace std;


class CUtility
{
    public :
        
    //Function return Person Loging to the system and executing the application     
    static std::string getCurrentUsername() {
    // Define a string to hold the username
    std::string username;

    // Platform-specific code to retrieve the username
    #if defined(_WIN32) || defined(_WIN64)
    // Windows
    char buffer[256];
    DWORD bufferSize = 256;
    if (GetUserNameA(buffer, &bufferSize)) {
        username = buffer;
    }
    #elif defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__)
    // Unix-like systems (Linux, macOS)
    char* loginName = getlogin();
    if (loginName != nullptr) {
        username = loginName;
    }
    #endif

    return username;
    }
        
        
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
        static bool isNumber(const std::string& str) {
         /*
            for (char c : str) {
            if (!std::isdigit(c)) {
                return false;
                }
            }
        return true; */
         return std::regex_match(str, std::regex("^\\[0-9]+$"));
        }

        static void OpenFileinBrowser(std::string sFileName)
        {
                std::string command = "start " + sFileName;
                std::system(command.c_str());
        }
        static void Printing(std::string sFileName)
        {
         
            HINSTANCE result = ShellExecuteA(NULL, "print", sFileName.c_str(), NULL, NULL, SW_HIDE);
            if (result > reinterpret_cast<HINSTANCE>(32)) {
                std::cout << "Printing started successfully.\n";
            } else {
                std::cerr << "Printing failed. Error code:\n";
            std::cout<<result;
            }
        }
        
};

class Logger {
private:
    std::ofstream logfile;
    std::string UserName;
public:
    
     // Disable Logger's copy assignment operator
    Logger& operator=(const Logger& other) = delete;

    Logger(const std::string& filename) {
        logfile.open(filename, std::ios_base::app); // Open file in append mode
        if (!logfile.is_open()) {
            std::cerr << "Error: Failed to open log file." << std::endl;
        }
        UserName = CUtility::getCurrentUsername();
        std::cout << "Current logged-in user: " << UserName << std::endl;
    }

    ~Logger() {
        if (logfile.is_open()) {
            logfile.close();
        }
    }

    void log(const std::string& message) {
        if (logfile.is_open()) {
            time_t now = time(0);
            tm* localTime = localtime(&now);
            char timestamp[100];
            strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", localTime);
            logfile <<UserName<<" "<< timestamp << " " << message << std::endl;
        } else {
            std::cerr << "Error: Log file is not open." << std::endl;
        }
    }
};



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
            throw std::runtime_error("Failed to connect to the database.");
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

                 std::string errorMessage = converter.to_bytes(message);
                 throw std::runtime_error("SQL Error: " + errorMessage);
            }
        }
        else
        {
            cout << "Datbase_Create_Update_Delete: Error in Initializing statement handle" << endl;
        }
        return retcode_stmt;
    }

    void Database_DisplayAll(const string& sqlSelect, std::string sRepoprtFileName="")
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


             
            if (retcode_stmt == SQL_SUCCESS || retcode_stmt == SQL_SUCCESS_WITH_INFO) 
            {
               
               /*
                SQLWCHAR SelectUserID[50];
                SQLWCHAR Selectname[50];
                SQLWCHAR Selectemail[50];
                //wcout << "<table border='1'>\n";
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
                std::string errorMessage = converter.to_bytes(message);
                throw std::runtime_error("SQL Error: " + errorMessage);
            
            }
            wcout << "</table>";
            */
                SQLSMALLINT numCols;
                SQLNumResultCols(sqlstatementhandle, &numCols);

                // Vector to hold the data for each row dynamically
                std::vector<std::vector<std::wstring>> rowsData;
                std::vector<std::string> columnNames;
                columnNames.clear();
                columnNames.reserve(numCols);

                for (SQLSMALLINT i = 1; i <= numCols; ++i) {
                    SQLCHAR columnName[256];
                    SQLSMALLINT columnNameLength;
                    SQLSMALLINT dataType;
                    SQLULEN columnSize;
                    SQLSMALLINT decimalDigits;
                    SQLSMALLINT nullable;

                    SQLDescribeCol(sqlstatementhandle, i, columnName, sizeof(columnName), &columnNameLength, &dataType, &columnSize, &decimalDigits, &nullable);
                    columnNames.emplace_back(reinterpret_cast<char*>(columnName));
                }   


                // Fetch rows
                while (SQLFetch(sqlstatementhandle) == SQL_SUCCESS) 
                {
                    // Vector to hold the data for the current row
                    std::vector<std::wstring> rowData;

                    // Retrieve data for each column in the row
                    for (SQLSMALLINT col = 1; col <= numCols; ++col) {
                        SQLWCHAR data[256]; // Adjust size as needed
                        SQLLEN lenOrInd = 0;
                        retcode_stmt = SQLGetData(sqlstatementhandle, col, SQL_C_WCHAR, data, sizeof(data), &lenOrInd);

                        // Check for NULL values or errors
                        if (retcode_stmt == SQL_NULL_DATA) {
                            rowData.push_back(L"NULL");
                        } else if (retcode_stmt == SQL_SUCCESS || retcode_stmt == SQL_SUCCESS_WITH_INFO) {
                            rowData.push_back(data);
                        } else {
                            // Handle error
                            SQLWCHAR sqlstate[6];
                            SQLWCHAR message[SQL_MAX_MESSAGE_LENGTH];
                            SQLINTEGER nativeError;
                            SQLSMALLINT messageLength;
                            SQLGetDiagRecW(SQL_HANDLE_STMT, sqlstatementhandle, 1, sqlstate, &nativeError, message, sizeof(message), &messageLength);
                            std::wstring errorMessage = std::wstring(message, messageLength);
                            throw std::runtime_error("SQL Error: " + converter.to_bytes(errorMessage));
                        }
                    }

                    // Add the current row's data to the rowsData vector
                    rowsData.push_back(rowData);
                }

                if(sRepoprtFileName !=""){
                
                    generateReportFile(sRepoprtFileName,columnNames,rowsData);
                    CUtility::OpenFileinBrowser(sRepoprtFileName+".csv");
                    CUtility::Printing(sRepoprtFileName+".csv");
                }
                else{
                    
                    // Print retrievedcolumn and  data
                    for(const auto& col: columnNames)
                    {
                        std::cout<<col<<"\t";
                    }
                    std::cout<< std::endl;
                    
                    for (const auto& row : rowsData) {
                        for (const auto& cell : row) {
                            std::wcout << cell << L"\t";
                        }
                        std::wcout << std::endl;
                    }
                
                }
            } 
            else 
            {
                // Handle SQL error
                SQLWCHAR sqlstate[6];
                SQLWCHAR message[SQL_MAX_MESSAGE_LENGTH];
                SQLINTEGER nativeError;
                SQLSMALLINT messageLength;
                SQLGetDiagRecW(SQL_HANDLE_STMT, sqlstatementhandle, 1, sqlstate, &nativeError, message, sizeof(message), &messageLength);
                std::wstring errorMessage = std::wstring(message, messageLength);
                throw std::runtime_error("SQL Error: " + converter.to_bytes(errorMessage));
            }
        }
        else
        {
            cout << "Database_DisplayAll: Error in Initializing statement handle" << endl;
        }
        
        // Free statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);

    }
    void generateReportFile( std::string sReportfileName, std::vector<std::string> columnNames, std::vector<std::vector<std::wstring>> rowsData)
    {
            std::ofstream outputFile(sReportfileName + ".csv", std::ios::app);
            if (!outputFile.is_open()) {
                    std::cerr << "Error: Failed to open file." << std::endl;
                }
            else
            {
                    std::cout<<"Report File Created:" +sReportfileName + ".csv"<<std::endl;
                    for(const auto& col: columnNames)
                    {
                        outputFile<<col;
                        outputFile<<"\t";
                        
                    }
                    outputFile<< std::endl;
                     std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    for (const auto& row : rowsData) 
                    {
                        for (const auto& cell : row) {
                            outputFile<<converter.to_bytes(cell);
                            outputFile<<"\t";
                        }
                        outputFile<<std::endl;
                    }

            }
              outputFile.close();       

    }

            
    int getIdenity()
    { 
        int lastInsertedId = -1; // Initialize to -1 to indicate no ID was retrieved        
        // Retrieve the last inserted ID
        retcode_stmt = SQLExecDirectW(sqlstatementhandle, (SQLWCHAR*)L"SELECT @@IDENTITY", SQL_NTS);
        if (retcode_stmt == SQL_SUCCESS || retcode_stmt == SQL_SUCCESS_WITH_INFO)
        {
            // Bind the result to a variable
            SQLINTEGER lastId;
            SQLLEN indicator;
            SQLBindCol(sqlstatementhandle, 1, SQL_C_SLONG, &lastId, sizeof(lastId), &indicator);

            // Fetch the result
            if (SQLFetch(sqlstatementhandle) == SQL_SUCCESS)
            {
                if (indicator != SQL_NULL_DATA)
                {
                    // Successfully retrieved the last inserted ID
                    lastInsertedId = lastId;
                    std::cout << "Last inserted ID: " << lastInsertedId << std::endl;
                }
                else
                {
                    // The last inserted ID is NULL
                    std::cout << "Last inserted ID is NULL" << std::endl;
                }
            }
            else
            {
                // Failed to fetch the result
                std::cerr << "Failed to fetch the last inserted ID" << std::endl;
            }
        }
        else
        {
            // Failed to execute the select statement for retrieving last ID
            std::cerr << "Failed to execute SELECT @@IDENTITY statement" << std::endl;
        }
        return lastInsertedId;
    }

};



Database* Database::instance = nullptr;


class User
{

private: 
     Logger objlogger;
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

    //User(){};
    ~User(){};

    User(const std::string& logFilename) : objlogger(logFilename) {}

    void logOperation(const std::string& operation) {
        objlogger.log("User operation: " + operation);
    }

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
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtInsert);
            logOperation("Create_User");
        }
        catch (const std::exception& e) {
            std::cerr << "Create_User: Error: " << e.what() << std::endl;
        }



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
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtUpdate);
            logOperation("Update_User");
        }
        catch (const std::exception& e) {
            std::cerr << "Update_User: Error: " << e.what() << std::endl;
        }
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
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtDelete);
            logOperation("Delete_User");
        }
        catch (const std::exception& e) {
            std::cerr << "Delete_User: Error: " << e.what() << std::endl;
        }
        
    }
    void DisplayAll()
    {
        std::string sqlStmtSelete = "Select UserID, Name, Email from Users;";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtSelete<<std::endl;
        try {
            DB->Database_DisplayAll(sqlStmtSelete);
            logOperation("DisplayAll");
        }
        catch (const std::exception& e) {
            std::cerr << "Users DisplayAll: Error: " << e.what() << std::endl;
        }


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
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtInsert);
        }
        catch (const std::exception& e) {
            std::cerr << "Create_Customers: Error: " << e.what() << std::endl;
        }
        
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
        try {
           std::cout << DB->Datbase_Create_Update_Delete(sqlStmtUpdate);
        }
        catch (const std::exception& e) {
            std::cerr << "Update_Customers: Error: " << e.what() << std::endl;
        }
        
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
        
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtDelete);

        }
        catch (const std::exception& e) {
            std::cerr << "Delete_Customers: Error: " << e.what() << std::endl;
        }
        
    }
    void DisplayAll()
    {
        std::string sqlStmtSelete = "Select * from Customers;";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtSelete<<std::endl;
        try {
            DB->Database_DisplayAll(sqlStmtSelete);

        }
        catch (const std::exception& e) {
            std::cerr << "Customers DisplayAll: Error: " << e.what() << std::endl;
        }
                    
    } 
};

class Video
{

private: 
     Logger objlogger;
/*
CREATE TABLE [dbo].[Video](
	[VideoID] [int] IDENTITY(1,1) NOT NULL,
	[VideoName] [nvarchar](50) NULL,
	[ActorName] [nvarchar](50) NULL,
	[ReleaseYear] [int] NULL,
 CONSTRAINT [PK_Vedios] PRIMARY KEY CLUSTERED 
(
	[VideoID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]

*/

public:
    int VideoID;
    std::string VideoName;
    std::string ActorName;
    int ReleaseYear;
    

    ~Video(){};

    Video(const std::string& logFilename) : objlogger(logFilename) {}

    void logOperation(const std::string& operation) {
        objlogger.log("Video operation: " + operation);
    }

    void Create_Video()
    {
        do{
                std::cout<<"Enter Video Name:";
                std::cin>>VideoName;
        }while(!CUtility::isValidName(VideoName));
        
        do{
                std::cout<<"Enter Actor Name:";
                std::cin>>ActorName;
        
        }while(!CUtility::isValidName(ActorName));

        do{
                std::cout<<"Enter Release Year:";
                std::cin>>ReleaseYear;
        
        }while(CUtility::isNumber(std::to_string(ReleaseYear)));

        /*
        std::string VideoName;
        std::string ActorName;
        int ReleaseYear;
        */

        std::string sqlStmtInsert = "Insert into Video(VideoName,ActorName,ReleaseYear) Values('" + VideoName + "','" + ActorName + "'," + std::to_string(ReleaseYear) + ");";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtInsert<<std::endl;
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtInsert);
            logOperation("Create_Video");
        }
        catch (const std::exception& e) {
            std::cerr << "Create_Video: Error: " << e.what() << std::endl;
        }



    }
    void Update_Video()
    {
        int iId;
        std::cout<<"Select the ID for Update"<<endl;
        DisplayAll();
        cin>>iId;
        std::string sVideoName, sActorName;
        int iReleaseYear;
        std::cout<<"Enter Video Name:";
        std::cin>>sVideoName;
        std::cout<<"Enter Actor Name:";
        std::cin>>sActorName;
        std::cout<<"Enter Release Year:";
        std::cin>>iReleaseYear;
        
        std::string sqlStmtUpdate = "Update Video SET VideoName='" + sVideoName +"',ActorName='" + sActorName + "',ReleaseYear=" + std::to_string(iReleaseYear) +" where VideoID=" + std::to_string(iId);
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtUpdate<<std::endl;
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtUpdate);
            logOperation("Update_Video");
        }
        catch (const std::exception& e) {
            std::cerr << "Update_Video: Error: " << e.what() << std::endl;
        }
    }
     void Delete_Video()
    {
        int iId;
        std::cout<<"Select the ID for Delete"<<endl;
        DisplayAll();
        cin>>iId;
        std::string sqlStmtDelete = "Delete From Video where VideoID=" + std::to_string(iId);
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtDelete<<std::endl;
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtDelete);
            logOperation("Delete_Video");
        }
        catch (const std::exception& e) {
            std::cerr << "Delete_Video: Error: " << e.what() << std::endl;
        }
        
    }
    void DisplayAll()
    {
        std::string sqlStmtSelete = "Select * from Video;";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtSelete<<std::endl;
        try {
            DB->Database_DisplayAll(sqlStmtSelete);
            logOperation("DisplayAll");
        }
        catch (const std::exception& e) {
            std::cerr << "Video DisplayAll: Error: " << e.what() << std::endl;
        }


    }        
};


class TransactionMaster {

/*
CREATE TABLE [dbo].[TransactionMaster](
	[TrID] [int] IDENTITY(1,1) NOT NULL,
	[TrDate] [date] NULL,
	[CustomerID] [int] NULL,
	[EmployeeID] [int] NULL,
 CONSTRAINT [PK_TransactionMaster] PRIMARY KEY CLUSTERED 
(
	[TrID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
GO
*/

public:
    int TrID;
    std::string TrDate;
    int CustomerID;
    int EmployeeID;

    TransactionMaster()=default;
    ~TransactionMaster() = default;

    void Create_TransactionMaster()
    {
        std::cout<<"Enter TrDate in YYYY-mm-DD Format:";
        std::cin>>TrDate;
        do{
                std::cout<<"Enter Customer ID:";
                std::cin>>CustomerID;
        }while(CUtility::isNumber(std::to_string(CustomerID)));
        
        do{
            
                std::cout<<"Enter Employee ID:";
                std::cin>>EmployeeID;
        
        }while(CUtility::isNumber(std::to_string(EmployeeID)));

        /*
        [TrID] [int] IDENTITY(1,1) NOT NULL,
	    [TrDate] [date] NULL,
	    [CustomerID] [int] NULL,
	    [EmployeeID] [int] NULL,
        */

        std::string sqlStmtInsert = "Insert into TransactionMaster(TrDate,CustomerID,EmployeeID) Values('" + TrDate + "'," + std::to_string(CustomerID) + "," + std::to_string(EmployeeID) + ");";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtInsert<<std::endl;
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtInsert);
            TrID = DB->getIdenity();
            cout<<std::endl<<"TransactionMaster ID:" <<TrID;
        }
        catch (const std::exception& e) {
            std::cerr << "Create_Video: Error: " << e.what() << std::endl;
        }

    }

    void Update_TransactionMaster()
    {
        int iId;
        std::cout<<"Select the ID for Update"<<endl;
        DisplayAll();
        cin>>iId;
        int iEmployeeID, iCustomerID;
        std::string sTrDate;
        std::cout<<"Enter Employee ID:";
        std::cin>>iEmployeeID;
        std::cout<<"Enter Customer ID :";
        std::cin>>iCustomerID;
        std::cout<<"Enter Tr Date:";
        std::cin>>sTrDate;
        
         /*
        [TrID] [int] IDENTITY(1,1) NOT NULL,
	    [TrDate] [date] NULL,
	    [CustomerID] [int] NULL,
	    [EmployeeID] [int] NULL,
        */
        std::string sqlStmtUpdate = "Update TransactionMaster SET TrDate='" + sTrDate +"',CustomerID=" + std::to_string(iCustomerID) + ",EmployeeID=" + std::to_string(iEmployeeID) +" where VideoID=" + std::to_string(iId);
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtUpdate<<std::endl;
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtUpdate);
           
        }
        catch (const std::exception& e) {
            std::cerr << "Update_TransactionMaster: Error: " << e.what() << std::endl;
        }
    }
     void Delete_TransactionMaster()
    {
        int iId;
        std::cout<<"Select the ID for Delete"<<endl;
        DisplayAll();
        cin>>iId;
        std::string sqlStmtDelete = "Delete From TransactionMaster where TrID=" + std::to_string(iId);
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtDelete<<std::endl;
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtDelete);
           
        }
        catch (const std::exception& e) {
            std::cerr << "Delete_TransactionMaster: Error: " << e.what() << std::endl;
        }
        
    }
    void DisplayAll()
    {
        std::string sqlStmtSelete = "Select * from TransactionMaster;";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtSelete<<std::endl;
        try {
            DB->Database_DisplayAll(sqlStmtSelete);
            
        }
        catch (const std::exception& e) {
            std::cerr << "TransactionMaster DisplayAll: Error: " << e.what() << std::endl;
        }
    }

 };


class TransactionDetails {
    
/*
CREATE TABLE [dbo].[TransactionDetails](
	[TrID] [int] NOT NULL,
	[VideoID] [int] NOT NULL,
	[VideoName] [nvarchar](50) NULL,
	[Rent] [float] NULL,
 CONSTRAINT [PK_TransactionDetails] PRIMARY KEY CLUSTERED 
(
	[TrID] ASC,
	[VideoID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]


*/

public:
    int TrID;
    int VideoID;
    std::string VideoName;
    double Rent;

    TransactionDetails()=default;
    ~TransactionDetails()=default;  

    void Create_TransactionDetails()
    {
        
        do{
                std::cout<<"Enter Video ID:";
                std::cin>>VideoID;
        }while(CUtility::isNumber(std::to_string(VideoID)));
        
        do{
                std::cout<<"Enter Video Name:";
                std::cin>>VideoName;

        
        }while(!CUtility::isValidName(VideoName));

        do{
                std::cout<<"Enter Rent:";
                std::cin>>Rent;
        }while(CUtility::isNumber(std::to_string(Rent)));

        /*
        [TrID] [int] NOT NULL,
	    [VideoID] [int] NOT NULL,
	    [VideoName] [nvarchar](50) NULL,
	    [Rent] [float] NULL,
    */

        std::string sqlStmtInsert = "Insert into TransactionDetails (TrID,VideoID,VideoName,Rent) Values('" + std::to_string(TrID) + "'," + std::to_string(VideoID) + ",'" + VideoName + "'," + std::to_string(Rent) + ");";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtInsert<<std::endl;
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtInsert);
            
        }
        catch (const std::exception& e) {
            std::cerr << "Create_TransacitonDetails: Error: " << e.what() << std::endl;
        }

    }
    void Delete_TransactionDetails()
    {
        int iId;
        std::cout<<"Select the ID for Delete"<<endl;
        DisplayAll();
        cin>>iId;
        std::string sqlStmtDelete = "Delete From TransactionDetails where TrID=" + std::to_string(iId);
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtDelete<<std::endl;
        try {
            std::cout << DB->Datbase_Create_Update_Delete(sqlStmtDelete);
        }
        catch (const std::exception& e) {
            std::cerr << "Delete_TransactionDetails: Error: " << e.what() << std::endl;
        }
        
    }
    void DisplayAll()
    {
        std::string sqlStmtSelete = "Select * from TransactionDetails;";
        Database* DB = Database::getInstance();
        std::cout<<sqlStmtSelete<<std::endl;
        try {
            DB->Database_DisplayAll(sqlStmtSelete);
      
        }
        catch (const std::exception& e) {
            std::cerr << "TransactionDetails DisplayAll: Error: " << e.what() << std::endl;
        }


    }

};

class Invoice {
private:
    std::vector<TransactionMaster> vtransactionMasters;
    std::vector<TransactionDetails> vtransactionDetails;
public:
    void addTransaction(const TransactionMaster& transactionMaster, const std::vector<TransactionDetails>& details) {
        vtransactionMasters.push_back(transactionMaster);
        for (const auto& detail : details) {
            vtransactionDetails.push_back(detail);
        }
    }

    void generateInvoice(const std::string& fileName) {
        std::ofstream outputFile(fileName);
        if (outputFile.is_open()) {
            for (const auto& transactionMaster : vtransactionMasters) {
                outputFile << "Transaction ID: " << transactionMaster.TrID << std::endl;
                outputFile << "Transaction Date: " << transactionMaster.TrDate << std::endl;
                outputFile << "Customer ID: " << transactionMaster.CustomerID << std::endl;
                outputFile << "Employee ID: " << transactionMaster.EmployeeID << std::endl;

                double totalRent = 0.0;
                for (const auto& detail : vtransactionDetails) {
                    if (detail.TrID == transactionMaster.TrID) {
                        outputFile << "Video ID: " << detail.VideoID << ", Video Name: " << detail.VideoName << ", Rent: $" << detail.Rent << std::endl;
                        totalRent += detail.Rent;
                    }
                }
                outputFile << "Total Rent: $" << totalRent << std::endl;
                outputFile << "--------------------------------------------" << std::endl;
            }
            outputFile.close();
            CUtility::OpenFileinBrowser(fileName);
            CUtility::Printing(fileName);
            std::cout << "Invoice generated successfully and saved as: " << fileName << std::endl;
        } else {
            std::cerr << "Unable to open file: " << fileName << std::endl;
        }
    }
};

class Reports
{
public:
    // Define the mapReports member variable
    std::map<std::string, std::string> mapReports;

    // Constructor
    Reports() {
        // Initialize mapReports with the desired values
        mapReports["Customers"] = "Select * from Customers;";
        mapReports["Employees"] = "Select * from Users;";
        mapReports["Video"] = "Select  * from Video;";
        mapReports["TrMaster"] = "Select  * from TransactionMaster;";
        mapReports["TrDetails"] = "Select  * from TransactionDetails;";
    }
    void DisplayAll(int Report_SrNo)
    {
   
         std::string sqlStmtSelete = "select * from Users;";  //default
         std::string sReportfileName="Users"; //default
        // Iterate through the map to find the element at the specified index
        int currentIndex = 0;
        for (auto it = mapReports.begin(); it != mapReports.end(); ++it) {
            if (currentIndex == Report_SrNo) {
              sqlStmtSelete  = it->second;
              sReportfileName = it->first;
                break;
            }
            currentIndex++;
        }
        Database* DB = Database::getInstance();
        try {
         
            DB->Database_DisplayAll(sqlStmtSelete,sReportfileName);
         }
        catch (const std::exception& e) {
            std::cerr << "Reports DisplayAll: Error: " << e.what() << std::endl;
        }
    }
    void CustomReportDisplayAll()
    {
         std::string sqlStmtSelete = "select * from Users;";  //default
         std::string sReportfileName="Users"; //default
         std::cout<<std::endl<<"Enter SQL Statement for Custom Report:";
         std::cin.ignore();
         getline(std::cin, sqlStmtSelete);
         std::cout<<"Enter Report FileName:";
         std::cin>>sReportfileName;
         Database* DB = Database::getInstance();
         try {
           DB->Database_DisplayAll(sqlStmtSelete,sReportfileName);
         }
        catch (const std::exception& e) {
            std::cerr << "Reports CustomReportDisplayAll: Error: " << e.what() << std::endl;
        }

    }

};

int main()
{
    int Option;
    do
    {
        std::cout << "Welcome to MiniProject" << std::endl;
        std::cout << "1. User Management" << std::endl;
        std::cout << "2. Customer Management" << std::endl;
        std::cout << "3. Video Management "<<std::endl;
        std::cout << "4. Invoice/Transaciton"<<std::endl;
        std::cout << "5. Standard Reports"<<std::endl;
        std::cout << "6. Custom Reports"<<std::endl;
        
        
        std::cout << "Enter your Option: ";
        std::cin >> Option;

        switch (Option)
        {
            case 5: //Standard Reports 
            {
                Reports objReports;
                int iSrNo=0;
                for (const auto& report :  objReports.mapReports) {
                   std::cout<<iSrNo++<<" "<<"Key:"<< report.first << "  Value:" << report.second <<std::endl;
                }
                std::cout<<std::endl<<"Select Reports Index No "<<std::endl;
                cin>>iSrNo;
                objReports.DisplayAll(iSrNo);

            }    
            break;
            case 6: //Custom Reports 
            {
                Reports objReports;
                objReports.CustomReportDisplayAll();

            }    
            break;
            case 4: //Invoice/Transaction 
            {
                    TransactionMaster master;
                    master.Create_TransactionMaster();

                    
                    TransactionDetails objdetails;
                    objdetails.TrID = master.TrID;
                    objdetails.Create_TransactionDetails();

                    std::vector<TransactionDetails> vdetails;
                    vdetails.push_back(objdetails);
                    
                    Invoice invoice;
                    invoice.addTransaction(master, vdetails);
                    invoice.generateInvoice("invoice.txt");
            }    
            break;    
            case 1: //User Management 
            {
                    std::cout << "Following are the Options for User Management" << std::endl;
                    std::cout << "1. Create " << std::endl;
                    std::cout << "2. Update " << std::endl;
                    std::cout << "3. Delete " << std::endl;
                    std::cout << "4. View " << std::endl;
                    std::cout << "Enter your Option: ";
                    std::cin >> Option;
                    User objUser("UserLogger.log");
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
            case 2: //Customer Management 
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
            }
            break; //close of Customer Management  
            case 3: //Video Managemt 
            {
                    std::cout << "Following are the Options for Video Management" << std::endl;
                    std::cout << "1. Create " << std::endl;
                    std::cout << "2. Update " << std::endl;
                    std::cout << "3. Delete " << std::endl;
                    std::cout << "4. View " << std::endl;
                    std::cout << "Enter your Option: ";
                    std::cin >> Option;
                    Video objVideo("VideoLogger.log");
                    switch(Option)
                    {
                        case 1: //Add New Record
                            do{
                                objVideo.Create_Video();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no: "<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;
                        case 2: //Update Record
                            do{
                                objVideo.Update_Video();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no: "<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;
                        case 3://Delete Record
                            do{
                               objVideo.Delete_Video();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no: "<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;
                        case 4://View Record
                            do{
                                objVideo.DisplayAll();
                                std::cout << "Would you like to continue? Press 1 for yes, 0 for no: "<<std::endl;
                                std::cin >> Option;
                            } while (Option);             
                            break;

                    } //Close of Operations for Video 
            }
            break; // Break for Video Management

        }
            std::cout << "Would you like to perform Other Management? Press 1 for yes, 0 for no: "<<std::endl;
            std::cin >> Option;
        } while (Option); 

    return 0;
}
