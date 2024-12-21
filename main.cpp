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

        char* Token{ std::strtok(const_cast<char*>(lMessage.c_str()), "\n") };
        std::vector<std::string> Tokens{};
        Tokens.reserve(100);
        for (size_t Iterator{}; Token != nullptr; ++Iterator)
        {
            Tokens.push_back(Token);
            Token = std::strtok(nullptr, "\n");
        }
        // CoutMutex.lock();
        // for (const auto& Iterator : Tokens)
        // {
        //     std::cout << Iterator << ' ';
        // }
        // std::cout << "\n MESSAGE READ \n";
        // CoutMutex.unlock();
        std::vector<std::string> PMP{};
        std::string ApiPath{}, lParams{}, Method{};
        if (Tokens[0].find('?') == Tokens[1].npos) //POST PUT ETC
        {   //Tokenize first string for method and path
            //Last second for Parameters
            Token = std::strtok(const_cast<char*>(Tokens[0].c_str()), " ");
            PMP.push_back(Token);
            Token = std::strtok(nullptr, " ");
            PMP.push_back(Token);
            Token = std::strtok(const_cast<char*>(Tokens[Tokens.size() - 1].c_str()), "&");
            for (;Token!= nullptr;)
            {
                PMP.push_back(Token);
                Token = std::strtok(nullptr, "&");
            }
            Method = PMP[0];
            ApiPath = PMP[1];
            for (size_t Index{2}; Index < PMP.size(); ++Index)
            {
                lParams += PMP[Index] + '\n';
            }
        }
        else //GET
        {   //Tokenize first string for method, path and params
            Token = std::strtok(const_cast<char*>(Tokens[0].c_str()), " ");
            for (;Token != nullptr;)
            {
                PMP.push_back(Token);
                Token = std::strtok(nullptr, " ?&");
            }
            if (PMP.size() < 4)
            {
                std::cout << "Something wrong\n";
                return;
            }
            ApiPath = PMP[1];
            lParams = PMP[2] + '\n' + PMP[3],
                Method = Tokens[0];
        }


        // std::cout << "\n NOT STANDART \n";
        // if (PathEnd == 0) //FUCK FAVICON
        // {
        //     lMessage = "/";
        //     lPointer->waitForDisconnected(30);
        //     lPointer->close();
        //     delete lPointer;
        //     SocketVector.pop_back();
        //     std::cout << "\nFUCK FAVICON\n";
        //     continue;
        // }

        Tokens.clear();
        std::cout << ApiPath << " - api path\n" << lParams << " - params\n";



        for (size_t Iterator{2}; Iterator < PMP.size(); ++Iterator)
        {
            Token = std::strtok(const_cast<char*>(PMP[Iterator].c_str()), "=");
            Params[Token]=std::strtok(nullptr, "=");
        }

        for (const auto& Iterator : Params)
        {
            std::cout << Iterator.first << ' ' << Iterator.second << '\n';
        }
        //Differentiate(ApiPath, Params, Method, lMessage);
        NewDifferentiate(ApiPath, Params, Method, lMessage);
        lPointer->write(lMessage.c_str());
        lPointer->waitForBytesWritten();
        std::cout << "\n MESSAGE SENT " << lMessage << '\n' ;
        lPointer->waitForDisconnected(300);
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
    MainMap["/Api/Artist/"] = {{"POST", {"Email", "Password", "PhoneNumber"}},
                             {"DELETE", {"Id"}}};
    MainMap["/Api/Auth/"] = {{"POST", {"Email", "Password"}}};

    AnswerMap["/Api/Artist/"] = {{"POST", {"Id"}},
                               {"DELETE", {}}};
    AnswerMap["/Api/Auth/"] = {{"POST", {"Id"}}};
    MainMap["Api/Artist/Releases/"] = {{"POST", {"Email", "Album", "Name"}}};
    AnswerMap["Api/Artist/Releases/"] = {{"POST", {}}};

    FunctionMap[std::make_pair("Api/Artist/", "POST")] =
        &newAddUser;
    FunctionMap[std::make_pair("Api/Auth/", "POST")] =
        &newAuthorizeUser;
    FunctionMap[std::make_pair("Api/Artist/", "DELETE")] =
        &newDeleteUser;
    FunctionMap[std::make_pair("Api/Artist/Releases/", "POST")] =
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
