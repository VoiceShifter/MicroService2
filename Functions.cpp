# include "Functions.hpp"
# include <QSqlDatabase>
# include <QSqlError>
# include <QSqlQuery>
# include <iostream>
# include <sstream>



std::unordered_map<API_PATH, std::unordered_map<API_METHOD, std::vector<API_PARAM>>> MainMap;
std::unordered_map<API_PATH, std::unordered_map<API_METHOD, std::vector<API_PARAM>>> AnswerMap;
std::map<std::pair<API_PATH, API_METHOD>, std::function<signed int
                                                        (std::map<std::string, std::string>&, std::vector<std::string>&)>> FunctionMap;

void NewDifferentiate(std::string& Path, std::map<std::string, std::string>& Params,
                      std::string& Method, std::string& ReturnMessage)
{
    signed int Error{0};
    if (MainMap.find(Path) == MainMap.end())
    {
        ReturnMessage = "No such path";
        Error = 1;
        goto End;
    }

    if (MainMap[Path].find(Method) == MainMap[Path].end())
    {
        ReturnMessage = "No such method";
        Error = 2;
        goto End;
    }
    if (MainMap[Path][Method].size() != Params.size())
    {
        ReturnMessage = "Wrong amount of params";
        Error = 3;
        goto End;
    }
    for (std::string const & vIterator : MainMap[Path][Method])
    {
        if (Params.find(vIterator) == Params.end())
        {
            ReturnMessage = "Wrong params";
            Error = 4;
        }
    }

End:
    std::vector<std::string> Returns;
    Returns.reserve(5);
    if (Error != 0)
    {
        Returns.push_back("-1");
        FalseReturn(ReturnMessage, Error);
    }
    else
    {

        CreateReturn(Returns, Path, Method, ReturnMessage,
                     FunctionMap[std::make_pair(Path, Method)](Params, Returns));
    }

}

signed int newPendingRequest(std::map<std::string, std::string> &Params, std::vector<std::string> &ReturnMessage)
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
        ReturnMessage.push_back(""); //lExec.lastError().text().toStdString()
        std::cout << ReturnMessage[0];
        return 400;
    }
    return 201;
}


signed int newAddUser(std::map<std::string, std::string>& Params, std::vector<std::string>& ReturnMessage)
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
        ReturnMessage.push_back(""); //lExec.lastError().text().toStdString()
        std::cout << ReturnMessage[0];
        return 400;
    }
    Query.str("");
    Query << "select UUID from Artists where Email = '" << Params["Email"] << "';";
    std::cout << Query.str() << " - this is a query\n";
    lExec.exec(Query.str().c_str());
    lExec.next();
    ReturnMessage.push_back(lExec.value(0).toString().toStdString());
    std::cout << "\nUSER ADDED\n";
    return 201;
}

signed int newDeleteUser(std::map<std::string, std::string>& Params, std::vector<std::string>& ReturnMessage)
{
    std::cout << "\nDeleting Artists\n";
    QSqlDatabase Users{QSqlDatabase::addDatabase("QSQLITE")};
    Users.setDatabaseName("Users.sqlite");
    if (!Users.open()) {
        qDebug() << Users.lastError().text();
    }
    QSqlQuery lExec;
    std::stringstream Query;
    Query << "update Artists set isDeleted = 1 where UUID = '" << Params["Id"] << "';";
    std::cout << '\n' << Query.str() << '\n';

    if (!lExec.exec(Query.str().c_str()))
    {
        //lExec.next();
        ReturnMessage.push_back("");
        std::cout << ReturnMessage[0];
        return 400;

    }
    return 204;
}

signed int newAuthorizeUser(std::map<std::string, std::string>& Params, std::vector<std::string>& ReturnMessage)
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
        ReturnMessage.push_back(lSelect.value(4).toByteArray().toStdString());
        return 202;
    }
    else
    {
        std::cout << "\nNOT AUTHORIZED\n";
        ReturnMessage.push_back("1");
        return 401;
    }
    return 202;
}

void CreateReturn(std::vector<std::string> &Answers, std::string& Path,
                  std::string& Method, std::string& ReturnMessage, signed int Code)
{
    std::stringstream Response{};
    // QHttpServerResponse qResponse{};
    // qResponse.addHeader("OK", 200);
    std::string Naming{};
    switch (Code) {
    case 200:
        Naming = "OK";
        break;
    case 201:
        Naming = "Created";
        break;
    case 202:
        Naming = "Accepted";
        break;
    case 204:
        Naming = "No content";
        break;
    case 400:
        Naming = "Bad request";
        break;
    case 401:
        Naming = "Unauthorized";
        break;
    case 404:
        Naming = "Not found";
        break;
    default:
        break;
    }
    Response << "HTTP/1.0 " << Code << ' ' << Naming << " \nContent-Type: application/json\n{\n";


    for (size_t Index{}; Index < Answers.size(); ++Index)
    {
        Response << '"' << AnswerMap[Path][Method][Index] << '"' << " : "
                 << '"' << Answers[Index] << '"' << '\n';
    }
    Response << "}\n";
    std::cout << Response.str() << '\n'; //remove later
    ReturnMessage = Response.str();
}

void FalseReturn(std::string &ReturnMessage, signed int State)
{
    std::stringstream Response{};
    Response << "HTTP/1.0 " << "404" << " Not found\nContent-Type: application/json\n{\n";
    switch (State) {
    case 1:
        Response << "\"Error\" : \"Wrong path\"\n" ;
        break;
    case 2:
        Response << "\"Error\" : \"Wrong Method\"\n" ;
        break;
    case 3:
        Response << "\"Error\" : \"Wrong Amount of params\"\n" ;
        break;
    case 4:
        Response << "\"Error\" : \"Wrong params\"\n" ;
        break;
    default:
        Response << "\"Error\" : \"How did you get here\"\n" ;
        break;
    }


    Response << "}\n";
    std::cout << Response.str(); //remove later
    ReturnMessage = Response.str();
}



