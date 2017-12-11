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

    // The data we are receiving
    ByteArray data;

    // Are we terminating?
    bool& terminate;
public:
    SocketThread1(DataHolder& d1, Socket& socket, bool& terminate)
    : d1(d1), socket(socket), terminate(terminate)
    {}

    ~SocketThread1()
    {}

    Socket& GetSocket()
    {
        return socket;
    }

    virtual long ThreadMain()
    {
        // If terminate is ever flagged, we need to gracefully exit
        while(!terminate)
        {
            try
            {
                // Wait for data
                socket.Read(data);

                // Perform operations on the data
                string data_str = data.ToString();
                cout << "socket1: " << data_str << endl;

                d1.setdata1(data_str);
                d1.sem1.Signal();

                cout << "Socket 1 says: " << data_str << endl;

                if (data_str == "dead"){
                    
                    cout << "I, socket 1, have died" << endl;
                    return 0;
                }

                d1.sem2.Wait();
                data_str = d1.getdata2();

                cout << "socket2data: " << data_str << endl;

                if (data_str == "dead"){

                    cout << "Socket 2 has died" << endl;
                    
                    d1.death.Trigger();
                    return 0;
                }


                data = ByteArray(data_str);

                // Send it back
                socket.Write(data);
            }
            catch (...)
            {
                // We catch the exception, but there is nothing for us to do with it here. Close the thread.
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

    // The data we are receiving
    ByteArray data;

    // Are we terminating?
    bool& terminate;
public:
    SocketThread2(DataHolder& d1, Socket& socket, bool& terminate)
    : d1(d1), socket(socket), terminate(terminate)
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
                // Wait for data
                socket.Read(data);
                

                // Perform operations on the data
                std::string data_str = data.ToString();
                cout << "socket2: " << data_str << endl;

                d1.setdata2(data_str);
                d1.sem2.Signal();

                if (data_str == "dead"){

                    cout << "I, socket 2, have died" << endl;
                    return 0;
                }

                d1.sem1.Wait();
                data_str = d1.getdata1();

                if (data_str == "dead"){
                    
                    d1.death.Trigger();
                    cout << "socket 1 has died" << endl;
                    return 0;
                }


                data = ByteArray(data_str);

                // Send it back
                socket.Write(data);
            }
            catch (...)
            {
                // We catch the exception, but there is nothing for us to do with it here. Close the thread.
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
    bool& terminate;

public:
    DataHolder d1;

    GameThread(ThreadSem& game, SocketServer& server,  bool& terminate)
    : game(game), server(server), terminate(terminate)
    {}

    ~GameThread()
    {

        // Terminate the thread loops
        terminate = true;
    }


    long ThreadMain()
    {
        while(true)
        {
            try
            {

                // Wait for a client socket connection
                Socket* newConnection1 = new Socket(server.Accept());

                // Pass a reference to this pointer into a new socket thread
                Socket& socketReference1 = *newConnection1;
                SocketThread1 s1(d1, socketReference1, terminate);

                 // Wait for a client socket connection
                 Socket* newConnection2 = new Socket(server.Accept());
                 
                // Pass a reference to this pointer into a new socket thread
                Socket& socketReference2 = *newConnection2;
                SocketThread2 s2(d1, socketReference2, terminate);

                game.Signal();
                d1.death.Wait();

                cout << "GameThread died" << endl;

                return 0;

            }
            catch (TerminationException terminationException)
            {
                return terminationException;
            }
            catch (std::string error)
            {
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
    ThreadSem game;

    ServerThread(SocketServer& server)
    : server(server),game()
    {}

    ~ServerThread()
    {
        cout << "Server thread closed down" << endl;   
        
        // Close the client sockets
        for (auto GameThread : SocketThreads)
        {
            
        }
        

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
                game.Wait();
            }
            catch (TerminationException terminationException)
            {
                return terminationException;
            }
            catch (std::string error)
            {
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
    server.Shutdown();
    return 0;
}
