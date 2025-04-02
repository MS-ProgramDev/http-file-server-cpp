#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <ctime>
#include "HttpRequest.h"
#include "HttpResponse.h"
using namespace std;

// Constants for server and sockets
static const size_t MAX_MESSAGE_SIZE = 4096; // Max size for request/response buffer
const int WEB_SERVER_PORT = 80;             // Port for the HTTP server
const int MAX_SOCKETS = 60;                 // Maximum number of simultaneous sockets
const int EMPTY = 0, LISTEN = 1, RECEIVE = 2, IDLE = 3, SEND = 4; // Socket states


// Structure to maintain socket state
struct SocketState
{
	SOCKET id;                        // Socket handle
	int recv;                         // Receiving state
	int send;                         // Sending state
	char buffer[MAX_MESSAGE_SIZE];    // Buffer for HTTP messages
	int len;                          // Length of data in the buffer
	time_t lastActivity;              // Timestamp of last socket activity
	bool closeAfterSend = false;      // Flag for closing connection after send
};

// Function declarations
bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);

// Array to store socket states
struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;

void main()
{
	
	WSAData wsaData;
	// Initialize Winsock
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Http Server: Error at WSAStartup()\n";
		return;
	}
	// Create a listening socket
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == listenSocket)
	{
		cout << "Http Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// Configure the server address
	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(WEB_SERVER_PORT);

	// Bind the socket to the port
	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Http Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	// Listen on the Socket for incoming connections.
	// This socket accepts only one connection (no pending connections 
	// from other clients). This sets the backlog parameter.
	//*backlog parameter is 5 mean 5 clients can wait in the same time
	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Http Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	// Add listening socket to the array
	addSocket(listenSocket, LISTEN); 

	// Accept connections and handles them one by one.
	while (true)
	{
		fd_set waitRecv;
		FD_ZERO(&waitRecv);

		// Populate the recv and send sets based on socket states
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv);
		}

		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}
		// Wait for activity on sockets
		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
		if (nfd == SOCKET_ERROR)
		{
			cout << "Http Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		// Timeout Check 120sec
		time_t currentTime = time(nullptr);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].recv != EMPTY && difftime(currentTime, sockets[i].lastActivity) > 120)
			{
				cout << "Http Server: Closing idle connection (timeout exceeded).\n";
				closesocket(sockets[i].id);
				removeSocket(i);
			}
		}

		// Handle recv activity
		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv))
			{
				nfd--;
				switch (sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(i);
					break;

				case RECEIVE:
					receiveMessage(i);
					break;
				}
			}
		}
		// Handle send activity
		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				nfd--;
				switch (sockets[i].send)
				{
				case SEND:
					sendMessage(i);
					break;
				}
			}
		}
	}

	// Closing connections and Winsock.
	cout << "Http Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

// Adds a new socket to the array
bool addSocket(SOCKET id, int what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			sockets[i].lastActivity = time(nullptr);
			return (true);
		}
	}
	return false;
}

// Removes a socket from the array
void removeSocket(int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	sockets[index].len = 0;
	socketsCount--;
}

// Accepts a new connection
void acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Http Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	//cout << "Http Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	//
	// Set the socket to be in non-blocking mode.
	//
	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "Http Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

// Handles incoming messages
void receiveMessage(int index)
{
	SOCKET msgSocket = sockets[index].id;
	int len = sockets[index].len;

	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len - 1, 0);
	if (bytesRecv == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK)
		{
			return;
		}
		cout << "Http Server: Error at recv(): " << error << endl;
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	if (bytesRecv == 0)
	{
		cout << "Http Server: Client disconnected.\n";
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	// Buffer Overflow Check
	if (sockets[index].len + bytesRecv >= sizeof(sockets[index].buffer))
	{
		cout << "Http Server: Buffer overflow detected. Closing connection.\n";
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}


	sockets[index].buffer[len + bytesRecv] = '\0'; // Null-terminate the string
	cout << "Http Server: Received: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";
	sockets[index].len += bytesRecv;
	//update last activity
	sockets[index].lastActivity = time(nullptr);

	// Parse and handle the request
	HttpRequest request;
	string rawRequest(sockets[index].buffer);
	bool parseSuccess = request.handleRequest(rawRequest);

	if (!parseSuccess)
	{
		// 400 Bad Request
		HttpResponse badRequest = HttpResponse::createBadRequestResponse();
		string httpResponse = badRequest.toString();
		memset(sockets[index].buffer, 0, sizeof(sockets[index].buffer));
		strncpy(sockets[index].buffer, httpResponse.c_str(), sizeof(sockets[index].buffer) - 1);
		sockets[index].len = (int)httpResponse.size();
		sockets[index].send = SEND;
		sockets[index].closeAfterSend = true; // Close after sending error response
		return;
	}

	// Generate response based on request
	HttpResponse response = request.handlePerMethodRequest();
	string httpResponse = response.toString();;
	memset(sockets[index].buffer, 0, sizeof(sockets[index].buffer));
	strncpy(sockets[index].buffer, httpResponse.c_str(), sizeof(sockets[index].buffer) - 1);
	sockets[index].len = (int)httpResponse.size();
	sockets[index].send = SEND;

	
	if (request.getHeaderConnection() == "close")
	{
		sockets[index].closeAfterSend = true; // Mark for closure
	}
}

// Sends a message to the client
void sendMessage(int index)
{
	SOCKET msgSocket = sockets[index].id;
	int bytesSent = send(msgSocket, sockets[index].buffer, sockets[index].len, 0);
	if (bytesSent == SOCKET_ERROR)
	{
		cout << "Http Server: Error at send(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	cout << "Http Server: Sent: " << bytesSent << " bytes of response.\n";

	// Check if the connection should be closed after sending
	if (sockets[index].closeAfterSend)
	{
		cout << "Http Server: Closing connection after send.\n";
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}

	// Reset the buffer and update the state
	memset(sockets[index].buffer, 0, sizeof(sockets[index].buffer));
	sockets[index].len = 0;
	sockets[index].send = IDLE;
}
