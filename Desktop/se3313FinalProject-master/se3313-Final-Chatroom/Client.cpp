
#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <string>

using namespace Sync;
using namespace std;
// This thread handles the connection to the server
class ClientThread : public Thread
{
private:
	// Reference to our connected socket
	Socket& socket;

	// Reference to boolean flag for terminating the thread
	bool& terminate;
	// Are we connected?
	bool connected = false;
	Event& termin;
	FlexWait fw;
	Blockable* temp;
	// Data to send to server
	ByteArray data;
	std::string data_str;
	int expectedLength = 0;
public:

	ClientThread(Socket& socket, bool& terminate, Event& termin)
	: socket(socket), terminate(terminate), termin(termin), fw(2, &socket, &termin)
	{}

	~ClientThread()
	{}

	void TryConnect()
	{
		try
		{
			std::cout << "Connecting...";
			std::cout.flush();
			int result = socket.Open();
			connected = true;
			std::cout << "OK" << std::endl;
		}
		catch (std::string exception)
		{
			std::cout << "FAIL (" << exception << ")" << std::endl;
			return;
		}
	}

	virtual long ThreadMain()
	{
		// Initially we need a connection
		while (true)
		{
			// Attempt to connect
			TryConnect();

			// Check if we are exiting or connection was established
			if (terminate || connected)
			{
				break;
			}

			// Try again every 5 seconds
			std::cout << "Trying again in 5 seconds" << std::endl;
			sleep(5);
		}

		while (!terminate)
		{

				data = ByteArray(data_str);
				temp = fw.Wait(-1);
                if(temp==&socket){
					socket.Read(data);
					string data_str = data.ToString();
					if(data_str=="dead"){
						terminate=true;
						return 0;
					}
					// If the length is different, the socket must be closed
				
						std::cout << "Server Response: " << data_str << std::endl;
					

					// Are we terminating
					if (terminate)
					{
						break;
					}
				}else if(temp==&termin){
					cout<<"termin triggered"<<endl;
					return 0;
				}
				else{
					cout<<"Colossal error"<<endl;
				}

			
			

		}

		return 0;
	}
};

int main(void)
{
	// Welcome the user and try to initialize the socket
	std::cout << "SE3313 Lab 4 Client" << std::endl;
			ByteArray data;
	// Create our socket
	Socket socket("127.0.0.1", 3000);
	bool terminate = false;
	Event termin;
	// Scope to kill thread
	{
		// Thread to perform socket operations on
		ClientThread clientThread(socket, terminate, termin);

		while(!terminate)
		{
				// We are connected, perform our operations
			std::cout << "Please input your data (done to exit): ";
			std::cout.flush();
			int expectedLength = 0;	
			// Get the data
			string data_str;
			data_str.clear();
			std::getline(std::cin, data_str);
			data = ByteArray(data_str);
			expectedLength = data_str.size();

			// Must have data to send
			if (expectedLength == 0)
			{
				std::cout << "Cannot send no data!" << std::endl;
				continue;
			}
			else if (data_str == "dead")
			{
				std::cout << "Closing the client..." << std::endl;
				sleep(3);
				termin.Trigger();
				terminate = true;
				break;
			}

			// Write to the server
			socket.Write(data);
		}
		
		// Wait to make sure the thread is cleaned up

	}
	sleep(1);
	socket.Close();

	
	return 0;
}
