# include <QCoreApplication>
# include <iostream>
# include <QtNetwork/QTcpServer>
# include <QtNetwork/QTcpSocket>
# include <QtNetwork/QHostAddress>
# include <QtCore>
# include <QTimer>
# include <chrono>
# include <thread>
# include <QSqlDatabase>
# include <QSqlError>
# include <QSqlQuery>
# include <map>
# include "Functions.hpp"
std::mutex CoutMutex{};

void ProcessVector(std::vector<QTcpSocket*>& SocketVector, std::map<std::string,
                                                                     std::string>& Params, std::string& lMessage)
{
    std::cout << "Thread started\n";
    using namespace std::chrono_literals;
    QTcpSocket* lPointer{};
    for (;;)
    {
        if (SocketVector.empty())
        {

            std::this_thread::sleep_for(500ms);
            CoutMutex.lock();
            std::cout << "Is empty\n";
            CoutMutex.unlock();
            continue;
        }
        std::cout << "SocketVector is not empty!\n";
        lPointer = SocketVector[SocketVector.size() - 1];
        if (!lPointer->waitForReadyRead()) {
            std::cerr << "Failed to read from socket: " << lPointer->errorString().toStdString() << std::endl;
            lPointer->close();
            delete lPointer;
            SocketVector.pop_back();
            continue;
        }
        QByteArray Data = lPointer->readAll();
        lMessage = QString::fromUtf8(Data).toStdString();
        std::cout << lMessage << '\n';

        char* Token{ std::strtok(const_cast<char*>(lMessage.c_str()), " ") };
        std::vector<std::string> Tokens{};
        Tokens.reserve(100);
        for (size_t Iterator{}; Token != nullptr; ++Iterator)
        {
            Tokens.push_back(Token);
            Token = std::strtok(nullptr, " ");
        }
        // CoutMutex.lock();
        // for (const auto& Iterator : Tokens)
        // {
        //     std::cout << Iterator << ' ';
        // }
        // std::cout << "\n MESSAGE READ \n";
        // CoutMutex.unlock();


        if (Tokens[1].size() < 2) //standart path
        {
            lMessage = "{\"This is a \": \"test path\"}";
            lPointer->write(lMessage.c_str());
            std::cout << "Standart path entered\n Message Sent\n";
            lPointer->waitForDisconnected(30);
            lPointer->close();
            delete lPointer;
            SocketVector.pop_back();
            std::cout << "\n CONNECTION TERMINATED \n";
            continue;
        }


        std::cout << "\n NOT STANDART \n";
        size_t PathEnd{Tokens[1].find_last_of('/')};
        if (PathEnd == 0) //FUCK FAVICON
        {
            lMessage = "/";
            lPointer->waitForDisconnected(30);
            lPointer->close();
            delete lPointer;
            SocketVector.pop_back();
            std::cout << "\nFUCK FAVICON\n";
            continue;
        }
        std::string ApiPath{Tokens[1].substr(1, PathEnd)},
            lParams{Tokens[1].substr(PathEnd + 2, Tokens[1].size())},
            Method{Tokens[0]};
        Tokens.clear();
        std::cout << ApiPath << " - api path\n" << lParams << " - params\n";

        Token = std::strtok(const_cast<char*>(lParams.c_str()), "&");
        for (size_t Iterator{}; Token != nullptr; ++Iterator)
        {
            Tokens.push_back(Token);
            Token = std::strtok(nullptr, "&");
        }

        for (size_t Iterator{0}; Iterator < Tokens.size(); ++Iterator)
        {
            Token = std::strtok(const_cast<char*>(Tokens[Iterator].c_str()), "=");
            Params[Token]=std::strtok(nullptr, "=");
        }

        for (const auto& Iterator : Params)
        {
            std::cout << Iterator.first << ' ' << Iterator.second << '\n';
        }
        //Differentiate(ApiPath, Params, Method, lMessage);
        NewDifferentiate(ApiPath, Params, Method, lMessage);
        lPointer->write(lMessage.c_str());
        std::cout << "\n MESSAGE SENT " << lMessage << '\n' ;
        lPointer->waitForDisconnected(30);
        lPointer->close();
        delete lPointer;
        SocketVector.pop_back();
        Params.clear();
        lMessage.clear();
        std::cout << "\n CONNECTION TERMINATED \n";
    }
}


int main(int argc, char *argv[])
{
    std::map<std::string, std::string> Params{};
    std::string ReturnMessage{};
    MainMap["Api/Artist/"] = {{"PUT", {"Email", "Password", "PhoneNumber"}},
                            {"GET", {"Email", "Password"}},
                            {"POST", {"Email", "Password"}}};
    MainMap["Api/Artist/Releases/"] = {{"POST", {"Email", "Album", "Name"}}};

    AnswerMap["Api/Artist/"] = {{"PUT", {"ErrorText", "Id"}},
                              {"GET", {"ErrorCode"}},
                              {"POST", {"ErrorCode"}}};
    AnswerMap["Api/Artist/Releases/"] = {{"POST", {"ErrorText"}}};


    FunctionMap[std::make_pair("Api/Artist/", "PUT")] =
        &newAddUser;
    FunctionMap[std::make_pair("Api/Artist/", "GET")] =
        &newAuthorizeUser;
    FunctionMap[std::make_pair("Api/Artist/", "POST")] =
        &newDeleteUser;
    FunctionMap[std::make_pair("Api/Artist/Releases", "POST")] =
        &newPendingRequest;



    std::vector<QTcpSocket*> Sockets{};
    QTcpServer MainSocket {};
    MainSocket.listen(QHostAddress::Any, 32324);
    std::mutex VectorMutex{};

    QObject::connect(&MainSocket, &QTcpServer::newConnection, [&MainSocket, &Sockets, &VectorMutex]()
                     {
                         VectorMutex.lock();
                         Sockets.push_back(MainSocket.nextPendingConnection());
                         VectorMutex.unlock();
                     });
    std::thread ProcessingThread(&ProcessVector, std::ref(Sockets), std::ref(Params), std::ref(ReturnMessage));
    ProcessingThread.detach();
    for(;;)
    {
        MainSocket.waitForNewConnection(-1);
    }
    std::cout << "Server is listening on port 32324\n";
    return 0;
}
