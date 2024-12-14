# include "Functions.hpp"
# include <QSqlDatabase>
# include <QSqlError>
# include <QSqlQuery>
# include <iostream>
# include <sstream>

std::unordered_map<API_PATH, std::unordered_map<API_METHOD, std::vector<API_PARAM>>> MainMap;
std::unordered_map<API_PATH, std::unordered_map<API_METHOD, std::vector<API_PARAM>>> AnswerMap;
std::map<std::pair<API_PATH, API_METHOD>, std::function<void
                                                        (std::map<std::string, std::string>&, std::vector<std::string>&)>> FunctionMap;

void NewDifferentiate(std::string& Path, std::map<std::string, std::string>& Params,
                      std::string& Method, std::string& ReturnMessage)
{
    signed int Error{0};
    if (MainMap.find(Path) == MainMap.end())
    {
        ReturnMessage = "No such path";
        return;
    }

    if (MainMap[Path].find(Method) == MainMap[Path].end())
    {
        ReturnMessage = "No such method";
        Error = 1;
    }
    if (MainMap[Path][Method].size() != Params.size())
    {
        ReturnMessage = "Wrong amount of params";
        Error = 2;
    }
    for (std::string const & vIterator : MainMap[Path][Method])
    {
        if (Params.find(vIterator) == Params.end())
        {
            ReturnMessage = "Wrong params";
            Error = 3;
        }
    }

    std::vector<std::string> Returns;
    Returns.reserve(5);
    if (Error != 0)
    {
        Returns.push_back("-1");
        FalseReturn(ReturnMessage, Error);
    }
    else
    {
        FunctionMap[std::make_pair(Path, Method)](Params, Returns);
        CreateReturn(Returns, Path, Method, ReturnMessage);
    }

}

void newPendingRequest(std::map<std::string, std::string> &Params, std::vector<std::string> &ReturnMessage)
{
    std::cout << "\nAdding new pending request\n";
    QSqlDatabase Users{QSqlDatabase::addDatabase("QSQLITE")};
    Users.setDatabaseName("Users.sqlite");
    if (!Users.open()) {
        qDebug() << Users.lastError().text();
    }
    QSqlQuery lExec;
    std::stringstream Query;
    Query << "Update Artists Set PendingRequest = PendingRequest + 1 Where Email = "
          << "'" << Params["Email"] << "';";
    std::cout << '\n' << Query.str() << '\n';
    if (!lExec.exec(Query.str().c_str()))
    {
        //lExec.next();
        ReturnMessage.push_back(lExec.lastError().text().toStdString());
        std::cout << ReturnMessage[0];
        return;
    }
    ReturnMessage.push_back("0");
    return;
}


void newAddUser(std::map<std::string, std::string>& Params, std::vector<std::string>& ReturnMessage)
{
    std::cout << "\nAdding new Artists\n";
    QSqlDatabase Users{QSqlDatabase::addDatabase("QSQLITE")};
    Users.setDatabaseName("Users.sqlite");
    if (!Users.open()) {
        qDebug() << Users.lastError().text();
    }
    QSqlQuery lExec;
    std::stringstream Query;
    Query << "INSERT INTO Artists (Email, PhoneNumber, Password) values ('" <<
        Params["Email"] << "', '" << Params["PhoneNumber"] << "', '" << Params["Password"] << "');";
    std::cout << '\n' << Query.str() << '\n';

    if (!lExec.exec(Query.str().c_str()))
    {
        //lExec.next();
        ReturnMessage.push_back(lExec.lastError().text().toStdString());
        std::cout << ReturnMessage[0];
        return;
    }
    ReturnMessage.push_back("0");
    Query.str("");
    Query << "select UUID from Artists where Email = '" << Params["Email"] << "';";
    std::cout << Query.str() << " - this is a query\n";
    lExec.exec(Query.str().c_str());
    lExec.next();
    ReturnMessage.push_back(lExec.value(0).toString().toStdString());
    std::cout << "\nUSER ADDED\n";
    return;
}

void newDeleteUser(std::map<std::string, std::string>& Params, std::vector<std::string>& ReturnMessage)
{
    std::cout << "\nDeleting Artists\n";
    QSqlDatabase Users{QSqlDatabase::addDatabase("QSQLITE")};
    Users.setDatabaseName("Users.sqlite");
    if (!Users.open()) {
        qDebug() << Users.lastError().text();
    }
    QSqlQuery lExec;
    std::stringstream Query;
    Query << "update Artists set isDeleted = 1 where Email = '" << Params["Email"]
          << "' and Password = '" << Params["Password"] << "';";
    std::cout << '\n' << Query.str() << '\n';

    if (!lExec.exec(Query.str().c_str()))
    {
        //lExec.next();
        ReturnMessage.push_back(lExec.lastError().text().toStdString());
        std::cout << ReturnMessage[0];

    }
    ReturnMessage.push_back("0");
    return;
}

void newAuthorizeUser(std::map<std::string, std::string>& Params, std::vector<std::string>& ReturnMessage)
{
    std::cout << "\nAuthorizing Artists\n";
    QSqlDatabase Users{QSqlDatabase::addDatabase("QSQLITE")};
    Users.setDatabaseName("Users.sqlite");
    if (!Users.open()) {
        qDebug() << Users.lastError().text();
    }
    QSqlQuery lSelect;
    std::stringstream Query;
    Query << "select * from Artists where Email = '"
          << Params.at("Email") << "' and Password = '" << Params.at("Password") << "'" << "and isDeleted = 0" << ";";
    std::cout << Query.str();

    lSelect.exec(Query.str().c_str());
    if (lSelect.next() == true)
    {
        std::cout << "\nUSER AUTHORIZED\n";
        ReturnMessage.push_back("0");
        return;
    }
    else
    {
        std::cout << "\nNOT AUTHORIZED\n";
        ReturnMessage.push_back("1");
        return;
    }
    return;
}

void CreateReturn(std::vector<std::string> &Answers, std::string& Path,
                  std::string& Method, std::string& ReturnMessage)
{
    std::stringstream Json{};
    Json << "{\n";
    for (size_t Index{}; Index < Answers.size(); ++Index)
    {
        Json << '"' << AnswerMap[Path][Method][Index] << '"' << " : "
             << '"' << Answers[Index] << '"' << '\n';
    }
    Json << '}';
    std::cout << Json.str(); //remove later
    ReturnMessage = Json.str();
}

void FalseReturn(std::string &ReturnMessage, signed int State)
{
    std::stringstream Json{};
    Json << "{\n";
    switch (State) {
    case 1:
        Json << "\"Error\" : \"Wrong path\"\n" ;
        break;
    case 2:
        Json << "\"Error\" : \"Wrong Method\"\n" ;
        break;
    case 3:
        Json << "\"Error\" : \"Wrong Params\"\n" ;
        break;
    default:
        Json << "\"Error\" : \"How did you get here\"\n" ;
        break;
    }
    Json << '}';
    std::cout << Json.str(); //remove later
    ReturnMessage = Json.str();
}



