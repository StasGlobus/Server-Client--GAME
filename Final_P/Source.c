#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include "Source.h"
#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"


void MainServer(int portnum)
{
	
	int Ind;
	int Loop;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	
	srand(time(NULL));

	HANDLE HELPT = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)HelperThread,
		NULL,
		0,
		NULL);
	SharedMutex = CreateMutex(
		NULL,
		FALSE,
		NULL);

	WSADATA wsaData;
	

	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}

	/* The WinSock DLL is acceptable. Proceed. */

	// Create a socket.    
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		goto server_cleanup_1;
	}

	// Bind the socket.
	/*
	For a server to accept client connections, it must be bound to a network address within the system.
	The following code demonstrates how to bind a socket that has already been created to an IP address
	and port.
	Client applications use the IP address and port to connect to the host network.
	The sockaddr structure holds information regarding the address family, IP address, and port number.
	sockaddr_in is a subset of sockaddr and is used for IP version 4 applications.
	*/
	// Create a sockaddr_in object and set its values.
	// Declare variables

	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		goto server_cleanup_2;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(portnum); //The htons function converts a u_short from host to TCP/IP network byte order 
									   //( which is big-endian ).
									   /*
									   The three lines following the declaration of sockaddr_in service are used to set up
									   the sockaddr structure:
									   AF_INET is the Internet address family.
									   "127.0.0.1" is the local IP address to which the socket will be bound.
									   2345 is the port number to which the socket will be bound.
									   */

									   // Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
									   // Check for general errors.
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;

	printf("Waiting for a client to connect...\n");
runnn:
	while (1)
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		//CleanupWorkerThreads();
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			goto server_cleanup_3;
		}

		printf("Client Connected.\n");

		Ind = FindFirstUnusedThreadSlot();

		if (Ind == NUM_OF_WORKER_THREADS) //no slot is available
		{
			printf("No slots available for client.\n");
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.
		}
		else
		{
			ThreadInputs[Ind] = AcceptSocket; // shallow copy: don't close 
											  // AcceptSocket, instead close 
											  // ThreadInputs[Ind] when the
											  // time comes.
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				&(ThreadInputs[Ind]),
				0,
				NULL
			);
		}
	} // for ( Loop = 0; Loop < MAX_LOOPS; Loop++ )

server_cleanup_3:

	CleanupWorkerThreads();
	goto runnn;
server_cleanup_2:
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static int FindFirstUnusedThreadSlot()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(ThreadHandles[Ind]);
				closesocket(ThreadInputs[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static void CleanupWorkerThreads()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0)
			{
				closesocket(ThreadInputs[Ind]);
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}

			else
			{
				//printf("Waiting for thread failed. Ending program\n");
				return;
			}
		}
	}
}

//recieve a pointer to the username as an argument and checks if the username is taken or not
char* handle_new_user(char *name) {
	char str[100];
	if (user_num == 0) {
		strcpy(current_name, name);
		user_num++;
		//sprintf(str, "NEW_USER_ACCEPTED:%d", user_num);
		sprintf(str, "SERVER_APPROVED");
		ApprovedName = 1;
		return str;
	}
	else
	{
		if (strcmp(current_name, name) == 0)
		{
			strcpy(str, "SERVER_DENIED");
			return str;
		}
		else {
			user_num++;
			//sprintf(str, "NEW_USER_ACCEPTED:%d", user_num);
			sprintf(str, "SERVER_APPROVED");
			ApprovedName = 1;
			return str;
		}
	}
}

//send string to all sockets
void SendStringMult(char *sendstr) {
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	char *AcceptedStr = NULL;
	int index = 0;
	WaitForSingleObject(SharedMutex, WAIT_TIME);
	for (index = 0; index < NUM_OF_WORKER_THREADS; index++)
		SendRes = SendString(sendstr, ThreadInputs[index]);
	ReleaseMutex(SharedMutex);
}
//checks if an error has occured during communication thorout the socket
int Check_Socket_Error(TransferResult_t RecvRes, SOCKET *t_socket) {
	if (RecvRes == TRNS_FAILED)
	{
		printf("Service socket error while reading, closing thread.\n");
		//closesocket(*t_socket);
		return 1;
	}
	else if (RecvRes == TRNS_DISCONNECTED)
	{
		printf("Connection closed while reading, closing thread.\n");
		//closesocket(*t_socket);
		return 1;
	}
	else
		return 0;
}



int game_res(char *user_0_sel, char *user_1_sel)
{
	if (STRINGS_ARE_EQUAL(user_0_sel, user_1_sel))
	{ return -1;}
	if (STRINGS_ARE_EQUAL(user_0_sel, "ROCK"))
	{
		if (STRINGS_ARE_EQUAL(user_1_sel, "PAPER"))
		{
			return 1;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "SCISSORS"))
		{
			return 0;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "LIZARD"))
		{
			return 0;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "SPOCK"))
		{
			return 1;
		}
	}
	if (STRINGS_ARE_EQUAL(user_0_sel, "PAPER"))
	{
		if (STRINGS_ARE_EQUAL(user_1_sel, "ROCK"))
		{
			return 0;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "SCISSORS"))
		{
			return 1;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "LIZARD"))
		{
			return 1;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "SPOCK"))
		{
			return 0;
		}
	}
	if (STRINGS_ARE_EQUAL(user_0_sel, "SCISSORS"))
	{
		if (STRINGS_ARE_EQUAL(user_1_sel, "PAPER"))
		{
			return 0;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "ROCK"))
		{
			return 1;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "LIZARD"))
		{
			return 0;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "SPOCK"))
		{
			return 1;
		}
	}
	if (STRINGS_ARE_EQUAL(user_0_sel, "LIZARD"))
	{
		if (STRINGS_ARE_EQUAL(user_1_sel, "PAPER"))
		{
			return 0;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "SCISSORS"))
		{
			return 1;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "ROCK"))
		{
			return 1;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "SPOCK"))
		{
			return 0;
		}
	}
	if (STRINGS_ARE_EQUAL(user_0_sel, "SPOCK"))
	{
		if (STRINGS_ARE_EQUAL(user_1_sel, "PAPER"))
		{
			return 1;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "SCISSORS"))
		{
			return 0;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "LIZARD"))
		{
			return 1;
		}
		if (STRINGS_ARE_EQUAL(user_1_sel, "ROCK"))
		{
			return 0;
		}
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Service thread is the thread that opens for each successful client connection and "talks" to the client.
//Helper thread helps send data to all the players in the game and manage the game.
static DWORD HelperThread() {
	char SendStr[SEND_STR_SIZE], SendStr1[SEND_STR_SIZE];
	char *AcceptedStr = NULL;
	BOOL Done = FALSE;
	int flag = 0;
	int tor = 0;
	int res = 2;
	char str[100];
	int turns[2] = { 1,2 };
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	while (1) {
		if (rdy == 0)
		{
			rdy = 2;
			printf("user0 - %s user 1 %s \n",clients_lst[0].username, clients_lst[1].username);
			strcpy(SendStr, "SERVER_INVITE:");
			strcat(SendStr, clients_lst[1].username);
			SendString(SendStr, ThreadInputs[0]);
			strcpy(SendStr, "SERVER_INVITE:");
			strcat(SendStr, clients_lst[0].username);
			SendString(SendStr, ThreadInputs[1]);

			SendStringMult("SERVER_PLAYER_MOVE_REQUEST");

		}
		if (selected == 0)
		{
			selected = 2; 
			rdy = 2;
			printf("user0 select - %s user1 select - %s \n", clients_lst[0].select, clients_lst[1].select);
			res = game_res(clients_lst[0].select, clients_lst[1].select);
			
				strcpy(SendStr, "SERVER_GAME_RESULTS:");
				strcat(SendStr, clients_lst[0].username);
				strcat(SendStr, ";");
				strcat(SendStr, clients_lst[1].select);
				strcat(SendStr, ";");
				strcat(SendStr, clients_lst[0].select);
				strcat(SendStr, ";");
				strcpy(SendStr1, "SERVER_GAME_RESULTS:");
				strcat(SendStr1, clients_lst[1].username);
				strcat(SendStr1, ";");
				strcat(SendStr1, clients_lst[0].select);
				strcat(SendStr1, ";");
				strcat(SendStr1, clients_lst[1].select);
				strcat(SendStr1, ";");
			if (res != -1)
			{
				strcat(SendStr, clients_lst[res].username);
				strcat(SendStr, ";");
				strcat(SendStr1, clients_lst[res].username);
				strcat(SendStr1, ";");
			}
			SendString(SendStr, ThreadInputs[1]);
			SendString(SendStr1, ThreadInputs[0]);
			SendStringMult("SERVER_GAME_OVER_MENU");
		}
	}
}

char* rand_cpu_res()
{
	int select = rand();
	select = select % 5;
	if (select == 0)
		return "ROCK";
	else if (select == 1)
		return "PAPER";
	else if (select == 2)
		return "SCISSORS";
	else if (select == 3)
		return "LIZARD";
	else
		return "SPOCK";
}

static DWORD ServiceThread(SOCKET *t_socket)
{
	char SendStr[SEND_STR_SIZE];
	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	char *AcceptedStr = NULL;
	char *token;
	char str[SEND_STR_SIZE];
	char p_name[SEND_STR_SIZE];
	char user_move[40];
	char* p_pointer;
	int player = 0;
	int turn = 1;
	int state = 0, user_ask_replay = 0;
	int ready, cpu_game_res;
	char *move;

	BOOL aginst_cpu;
	char *cpu_select, *user_select;
	AcceptedStr = NULL;
	RecvRes = ReceiveString(&AcceptedStr, *t_socket);
	state = Check_Socket_Error(RecvRes, &t_socket);
	if (state == 1)
		{
			printf("Player disconnected. Ending communication.");
			
			return 0;
		}
	else
		{
			strcpy(str, AcceptedStr);
			printf("Got string : %s\n", AcceptedStr);
		}
	/* get the first token */
	token = strtok(str, ":");
	if (STRINGS_ARE_EQUAL(token, "CLIENT_REQUEST"))
		{
			WaitForSingleObject(SharedMutex, WAIT_TIME); // test hre
			token = strtok(NULL, ";");
			strcpy(p_name, token);
			ApprovedName = 0;
			strcpy(SendStr, handle_new_user(token));
			player = user_num - 1;
			clients_lst[player].ClientNumber = player;
			strcpy(clients_lst[player].username, token);
			if (ApprovedName == 1) {
				if (user_num == 1)
					strcpy(p_name1, p_name);
				if (user_num == 2)
					strcpy(p_name2, p_name);
			}
			SendRes = SendString(SendStr, *t_socket);
			if (ApprovedName == 1) {
				SendRes = SendString("SERVER_MAIN_MENU", *t_socket);
			}
			ReleaseMutex(SharedMutex);
		}
	while (1)
		{
			AcceptedStr = NULL;
			RecvRes = ReceiveString(&AcceptedStr, *t_socket);
			state = Check_Socket_Error(RecvRes, &t_socket);
			if (state == 1)
			{
			
				printf("Player disconnected. Ending communication.\n");
			
			}
		
			printf("Got string : %s\n", AcceptedStr);
			if (STRINGS_ARE_EQUAL(AcceptedStr, "CLIENT_VERSUS"))
			{
				rdy--;
				aginst_cpu = FALSE;
			}
			if (STRINGS_ARE_EQUAL(AcceptedStr, "CLIENT_REPLAY"))
			{
				if (aginst_cpu == TRUE)
				{
					user_ask_replay = 1;
				}
				else
				{
					rdy--;
				}
			}
			if (STRINGS_ARE_EQUAL(AcceptedStr, "CLIENT_MAIN_MENU"))
			{
			
				SendRes = SendString("SERVER_MAIN_MENU", *t_socket);
				
			}
			if (STRINGS_ARE_EQUAL(AcceptedStr, "CLIENT_CPU") || user_ask_replay == 1)
			{
				user_ask_replay = 0;
				SendRes = SendString("SERVER_PLAYER_MOVE_REQUEST", *t_socket);
				aginst_cpu = TRUE;
			}
			token = strtok(AcceptedStr, ":");
			if (STRINGS_ARE_EQUAL(token, "CLIENT_PLAYER_MOVE"))
			{
				printf("in CLIENT_PLAYER_MOVE handle");
				if (aginst_cpu == TRUE )
				{
					user_select = strtok(NULL, ";");
					cpu_select = rand_cpu_res();
					printf("cpu_select: %s", cpu_select);
					strcpy(SendStr, "SERVER_GAME_RESULTS:");
					strcat(SendStr, "Server;");
					strcat(SendStr, user_select);
					printf("user_select is %s\n", user_select);
					strcat(SendStr, ";");
					strcat(SendStr, cpu_select);
					strcat(SendStr, ";");
					cpu_game_res = game_res(user_select, cpu_select);
					if (cpu_game_res == 0)
					{
						strcat(SendStr, clients_lst[player].username);
						strcat(SendStr, ";");
					}
					if (cpu_game_res == 1)
					{
						strcat(SendStr, "Server");
						strcat(SendStr, ";");
					}
					printf(SendStr, "\n");
					SendRes = SendString(SendStr, *t_socket);
					SendRes = SendString("SERVER_GAME_OVER_MENU", *t_socket);
				}
				else
				{
					user_select = strtok(NULL, ";");
					strcpy(clients_lst[player].select, user_select);
					printf("user select is: %s", clients_lst[player].select);
					selected--;
				}
			}
		}	
}