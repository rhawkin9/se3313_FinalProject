#include <iostream>
#include "thread.h"
#include "socketserver.h"
#include "Blockable.h"
#include <stdlib.h>
#include <algorithm>
#include <time.h>
#include <list>
#include <vector>
#include <string>

using namespace Sync;
using namespace std;

class DataHolder{
private:
    string data1;
    string data2;
public: 

    DataHolder(){}


    ThreadSem sem1;
    ThreadSem sem2;
    Event death;

    string getdata1(){
        
        return data1;
    }

    string getdata2(){

        return data2;
    }

    void setdata1(string data){

        data1 = data;
    }

    void setdata2(string data){
        
        data2 = data;
    }




};


// This thread handles each client connection
class SocketThread1 : public Thread
{
private:

    DataHolder& d1;
    // Reference to our connected socket
    Socket& socket;
    Blockable* temp;
    FlexWait fw;
    // The data we are receiving
    ByteArray data;

    // Are we terminating?
    bool& terminate;
public:
    SocketThread1(DataHolder& d1, Socket& socket, bool& terminate)
    : d1(d1), socket(socket), terminate(terminate), fw(2, &d1.sem2, &socket)
    {}

    ~SocketThread1()
    {}

    Socket& GetSocket()
    {
        return socket;
    }

    
    long ThreadMain()
    {
        // If terminate is ever flagged, we need to gracefully exit
        while(!terminate)
        {    
            try
            {

                string data_str;
                // Wait for data
                temp = fw.Wait(-1);
                if(temp==&d1.sem2){
                    cout<<"Sem2 called"<<endl;
                    d1.sem2.Wait();
                    data_str = d1.getdata2();

                    cout << "socket2data: " << data_str << endl;

                    if (data_str == "dead"){

                        cout << "Socket 2 has died" << endl;
                         data = ByteArray(data_str);

                        // Send it back
                        socket.Write(data);

                        d1.death.Trigger();
                        return 0;
                    }


                    data = ByteArray(data_str);

                    // Send it back
                    socket.Write(data);

                
                }else if(temp==&socket){
                    cout<<"Socket called"<<endl;
                    socket.Read(data);
                     data_str = data.ToString();
                    cout << "socket1: " << data_str << endl;

                    d1.setdata1(data_str);
                    d1.sem1.Signal();

                    cout << "Socket 1 says: " << data_str << endl;
                    if (data_str == "dead"){
                    
                        cout << "I, socket 1, have died" << endl;
                        return 0;
                    }
                
                }else{
                    cout<<"Good God"<<endl;
                }
                
            }
            catch (...)
            {
                // We catch the exception, but there is nothing for us to do with it here. Close the thread.
                cout << "Socket 1 catches itself" << endl;
            }
        }

        return 0;
    }
};

// This thread handles each client connection
class SocketThread2 : public Thread
{
private:

    DataHolder& d1;
    // Reference to our connected socket
    Socket& socket;
    Blockable* temp;
    FlexWait fw;
    // The data we are receiving
    ByteArray data;

    // Are we terminating?
    bool& terminate;
public:
    SocketThread2(DataHolder& d1, Socket& socket, bool& terminate)
    : d1(d1), socket(socket), terminate(terminate), fw(2, &d1.sem1, &socket)
    {}

    ~SocketThread2()
    {}

    Socket& GetSocket()
    {
        return socket;
    }

    long ThreadMain()
    {
        // If terminate is ever flagged, we need to gracefully exit
        while(!terminate)
        {
            try
            { 
                
                string data_str;
                temp = fw.Wait(-1);
                if(temp==&d1.sem1){
                    cout<<"Sem1 called"<<endl;
                    d1.sem1.Wait();
                    data_str = d1.getdata1();

                    cout << "socket1data: " << data_str << endl;

                    if (data_str == "dead"){

                        cout << "Socket 1 has died" << endl;
                       data = ByteArray(data_str);

                    // Send it back
                    socket.Write(data);
                        d1.death.Trigger();
                        return 0;
                    }


                    data = ByteArray(data_str);

                    // Send it back
                    socket.Write(data);

                
                }else if(temp==&socket){
                    cout<<"Socket called"<<endl;
                    socket.Read(data);
                     data_str = data.ToString();
                    cout << "socket2: " << data_str << endl;

                    d1.setdata2(data_str);
                    d1.sem2.Signal();

                    cout << "Socket 2 says: " << data_str << endl;
                    if (data_str == "dead"){
                    
                        cout << "I, socket 2, have died" << endl;
                        return 0;
                    }
                
                }else{
                    cout<<"Good God"<<endl;
                }
                
            
                
            }
            catch (...)
            {
                // We catch the exception, but there is nothing for us to do with it here. Close the thread.
                cout << "Socket 2 catches itself" << endl;
            }
        }

        return 0;
    }
};

// This thread handles the server operations
class GameThread : public Thread
{
private:
    ThreadSem& game;
    SocketServer& server;
    std::vector<SocketThread1*> SocketThread1s;
    std::vector<SocketThread2*> SocketThread2s;
    bool& terminate;

public:
    DataHolder d1;

    GameThread(ThreadSem& game, SocketServer& server,  bool& terminate)
    : game(game), server(server), terminate(terminate)
    {}

    ~GameThread()
    {

        cout << "GameThread's destructor" << endl;
        d1.death.Trigger();
        cout <<"Death triggered"<<endl;
        // Close the client socket 1
        for (auto SocketThread1 : SocketThread1s)
        {
            try
            {
                cout<<"Attempted socket 1 closure"<<endl;
                // Close the socket
                Socket& toClose = SocketThread1->GetSocket();
                toClose.Close();
                cout << "Socket 1 has closed and died a warrior's death" << endl;
            }
            catch (...)
            {
                // If already ended, this will cause an exception
                cout << "Socket 1 catches in gamethread" << endl;
            }
        }

        // Close the client socket 2
        for (auto SocketThread2 : SocketThread2s)
        {
            try
            {
                // Close the socket
                Socket& toClose = SocketThread2->GetSocket();
                toClose.Close();
                cout << "Socket 2 has closed and died a warrior's death" << endl;
            }
            catch (...)
            {
                // If already ended, this will cause an exception
                cout << "Socket 2 catches in gamethread" << endl;
            }
        }
        
        

        // Terminate the thread loops
        terminate = true;
    }


    long ThreadMain()
    {
        while(!terminate)
        {
            try
            {

                // Wait for a client socket connection
                Socket* newConnection1 = new Socket(server.Accept());

                // Pass a reference to this pointer into a new socket thread
                Socket& socketReference1 = *newConnection1;
                SocketThread1s.push_back(new SocketThread1(d1, socketReference1, terminate));
                cout<<"created socket1"<<endl;
                // Wait for a client socket connection
                Socket* newConnection2 = new Socket(server.Accept());
                 
                // Pass a reference to this pointer into a new socket thread
                Socket& socketReference2 = *newConnection2;
                SocketThread2s.push_back(new SocketThread2(d1, socketReference2, terminate));
                cout<<"created socket2"<<endl;
                game.Signal();
                d1.death.Wait();

                cout << "GameThread died" << endl;

                return 0;

            }
            catch (TerminationException terminationException)
            {
                cout << "Terimnation exception in gamethread" << endl;
                return terminationException;
            }
            catch (std::string error)
            {
                cout << "catch std::string error gamethread" << endl;
                std::cout << std::endl << "[Error] " << error << std::endl;
                return 1;
            }
        }
    }
};


// This thread handles the server operations
class ServerThread : public Thread
{
private:
    SocketServer& server;
    std::vector<GameThread*> SocketThreads;
    bool terminate = false;
public:
    Blockable* temp;
    ThreadSem game;
    Event wrapItUp;
    FlexWait fw;

    ServerThread(SocketServer& server)
    : server(server),game(), wrapItUp(), fw(2, &game, &wrapItUp)
    {}

    ~ServerThread()
    {
        cout << "Server thread closed down" << endl;   
        

        // Terminate the thread loops
        terminate = true;
    }

    long ThreadMain()
    {
        while(true)
        {
            try
            {
                cout << "Created new GameThread!" << endl;
                
                // Pass a reference to this pointer into a new socket thread
               SocketThreads.push_back(new GameThread(game,server,terminate));
                //game.Wait();
                temp=fw.Wait();
                if(temp==&wrapItUp){
                    cout<<"Wrapped it up"<<endl;
                    return 0;
                }else if(temp==&game){
                    cout<<"Game called"<<endl;
                    game.Wait();

                }else{
                    cout<<"Good God"<<endl;
                }

            }
            catch (TerminationException terminationException)
            {
                cout << "Serverthread termination exception" << endl;
                return terminationException;
            }
            catch (std::string error)
            {
                cout << "Serverthread std::string error catch" << endl;
                std::cout << std::endl << "[Error] " << error << std::endl;
                return 1;
            }
        }
    }
};

int main(void)
{
    // Welcome the user
    std::cout << "SE3313 Final Project Server" << std::endl;
    std::cout << "Press enter to terminate the server...";
    std::cout.flush();

    // Create our server
    SocketServer server(3000);    

    // Need a thread to perform server operations
    ServerThread serverThread(server);

    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    std::cin.get();




    // Shut down and clean up the server
    serverThread.wrapItUp.Trigger();
    server.Shutdown();
    sleep(1);
 


     
    return 0;
}
