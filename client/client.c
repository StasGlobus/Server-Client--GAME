#define _CRT_SECURE_NO_WARNINGS
#include "client.h"
#include "SocketSendRecvTools.h"



//function for recieving string from server.

static DWORD RecvDataThread(void)
{
	char RecievedString[MAX_MESSAGE_LINE];
	char newRecievedString[MAX_MESSAGE_LINE];
	char* StringPharser;
//	str BoardStr;
	char *AcceptedStr = NULL;
//	char WinnerName[MAX_MESSAGE_LINE];
//	int NameOrState;
//	int i, j;
	int ind = 0;
	//int Val;
	int first_time = 0;
	int already_connected = 0, sent_menu = 0;
	TransferResult_t RecvRes;
	char *token;


	while (ThreadEnd == 0)
	{
		if (1)
		{
			AcceptedStr = NULL;
			RecvRes = ReceiveString(&AcceptedStr, m_socket);
			strcpy(newRecievedString, AcceptedStr);
			if (RecvRes == TRNS_FAILED)
			{
				if (sent_menu == 0 ) 
				{
					sent_menu = 1;
					menu = 0;
					printf("Connection to server %s:%s as been lost.\n","127.0.0.1","777");
					printf("Choose what to do next:\n");
					printf("1. Try to reconnect\n");
					printf("2. Exit\n");
				}
			}
			else {
				sent_menu = 0;
				first_time = 1;
				StringPharser = strtok(AcceptedStr, ":");
				strcpy(RecievedString, StringPharser);
				printf("Got string: %s\n", AcceptedStr);

				//handle with differant messages from the server:
				//SERVER_APPROVED
				if (strcmp(RecievedString, "SERVER_APPROVED") == 0)
				{
					printf("%s Was Approved\n", Client.username);
				}

				if ((strcmp(RecievedString, "SERVER_MAIN_MENU") == 0) || (strcmp(RecievedString, "SERVER_NO_OPPONENTS") == 0))
				{
					menu = 1;
					if (strcmp(RecievedString, "SERVER_NO_OPPONENTS") == 0)
						printf("No Opponents was Found:\n");

					printf("Choose what to do next:\n");
					printf("1. Play against another client\n");
					printf("2. Play against the server\n");
					printf("3. Quit\n");
				}
					
				//SERVER_DENIED
				if (strcmp(RecievedString, "SERVER_DENIED") == 0)
				{
					ThreadEnd = 1;
					printf("Request to join was refused\n");
					ThreadEnd = 1;
					TerminateThread(hThread[0], 0x555);
					TerminateThread(hThread[2], 0x555);
					TerminateThread(hThread[1], 0x555);
					CloseHandle(hThread[0]);
					CloseHandle(hThread[1]);
					CloseHandle(hThread[2]);
					return 0x555;
					//close everything
				}
				if (strcmp(RecievedString, "SERVER_PLAYER_MOVE_REQUEST") == 0)
				{
					printf("Choose a move from the list: Rock, Paper, Scissors, Lizard or Spock:\n");
					menu = 2;
				}

				if (strcmp(RecievedString, "SERVER_GAME_RESULTS") == 0)
				{
					char *opp_name, *my_sel, *opp_sel, *winner;
					token = strtok(newRecievedString, ":");

					opp_name = strtok(NULL, ";");
					my_sel = strtok(NULL, ";");
					opp_sel = strtok(NULL, ";");
					printf("You played: %s\n", my_sel);
					printf("%s played: %s\n", opp_name, opp_sel);
					winner = strtok(NULL, ";");
					if (winner != NULL)
						printf("%s won!\n", winner);


				}
				if (strcmp(RecievedString, "SERVER_GAME_OVER_MENU") == 0)
				{
					menu = 3;
					printf("Choose what to do next:\n");
					printf("1. Play again\n");
					printf("2. Return to the main menu\n");
				}
				if (strcmp(RecievedString, "SERVER_OPPONENT_QUIT") == 0)
				{
					token = strtok(newRecievedString, ":");
					token = strtok(NULL, ";");
					printf("%s has left the game:\n",token);
				}
				if (strcmp(RecievedString, "SERVER_INVITE") == 0)
				{
					token = strtok(NULL, ";");
					printf("Game against %s is about to start!\n", token);
				}
				Client.CanWrite = 1;
			}		
		}

	}

	return 0;
}

int check_valid_move(char *user_str) 
{
	if (STRINGS_ARE_EQUAL(user_str, "ROCK") || STRINGS_ARE_EQUAL(user_str, "PAPER") || STRINGS_ARE_EQUAL(user_str, "SCISSORS") || STRINGS_ARE_EQUAL(user_str, "LIZARD") || STRINGS_ARE_EQUAL(user_str, "SPOCK")) 
		return 1;
	return 0;
}

//function for sending string to server

static DWORD SendDataThread(void)		//this is the consumer.
{
	char SendStr[MAX_MESSAGE_LINE];
	char ReadFromBufferString[MAX_MESSAGE_LINE];
//	char* token;
	int first_read = 0;
	int error = 0;
//	int boolian;
	int first_entrance = 0, move_res;
	TransferResult_t SendRes;
	SOCKADDR_IN clientService;

	DWORD wait_res;
	BOOL release_res;

	strcpy(SendStr, "CLIENT_REQUEST:");
	strcat(SendStr, Client.username);
	SendRes = SendString(SendStr, m_socket);



	while (ThreadEnd == 0)
	{

		strcpy(ReadFromBufferString, "");
		wait_res = WaitForSingleObject(mutex_buffer, INFINITE); //chenged timing here
		if (wait_res != WAIT_OBJECT_0) ReportErrorAndEndProgram();


		//boolian = ((Client.my_turn == 1) && (Client.PlayerMode == 0));
		/* Start Critical Section - reading from messages buffer*/
		if ((Client.CanRead == 1) && (send_new_string == 1))
		{
			//printf("New line from user: %s", messages_to_server);
			strcpy(ReadFromBufferString, messages_to_server);
			Client.CanRead = 0;
			//released critical section. now dealling with differnet types of messeges
			if (first_write)
			{
				if (start || (first_entrance == 0))
				{	
					first_entrance = 1;
					if (menu == 0)
					{
						if (STRINGS_ARE_EQUAL(ReadFromBufferString, "1"))
						{
							m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

							// Check for errors to ensure that the socket is a valid socket.
							if (m_socket == INVALID_SOCKET) {
								printf("Error at socket(): %ld\n", WSAGetLastError());
								WSACleanup();
								return;
							}


							//Create a sockaddr_in object clientService and set  values.
							//clientService.sin_family = AF_INET;
							//clientService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR); //Setting the IP address to connect to
							//clientService.sin_port = htons(777); //Setting the port to connect to.
							//while (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
						//		printf("Failed connection to server on 127.0.0.1:%d.Exiting.\n", 777);
								//WSACleanup();
						//		Sleep(3000);
							//	return;
						//	}
						//	printf("Connected to server on 127.0.0.1:%d\n", 777);

						}

						else if (STRINGS_ARE_EQUAL(ReadFromBufferString, "2"))
						{
							ThreadEnd = 1;
							TerminateThread(hThread[0], 0x555);
							TerminateThread(hThread[2], 0x555);
							release_res = ReleaseMutex(mutex_buffer);
							if (release_res == FALSE) ReportErrorAndEndProgram();
							TerminateThread(hThread[1], 0x555);
							CloseHandle(hThread[0]);
							CloseHandle(hThread[1]);
							CloseHandle(hThread[2]);
							return 0x555; //"exit" signals an exit from the client side
						}
					}
					else if (menu == 1)
					{
						if (STRINGS_ARE_EQUAL(ReadFromBufferString, "1")) 
						{
							strcpy(SendStr, "CLIENT_VERSUS");
							printf("Player Choose to play aginst other player");
							error = 0;
							start = 1;
						}

						else if (STRINGS_ARE_EQUAL(ReadFromBufferString, "2"))
						{
							strcpy(SendStr, "CLIENT_CPU");
							error = 0;
							start = 1;
						}

						else if (STRINGS_ARE_EQUAL(ReadFromBufferString, "3"))
						{
							ThreadEnd = 1;
							//TerminateThread(hThread[0], 0x555);
							//TerminateThread(hThread[2], 0x555);
							release_res = ReleaseMutex(mutex_buffer);
							if (release_res == FALSE) ReportErrorAndEndProgram();
							TerminateThread(hThread[1], 0x555);
							CloseHandle(hThread[0]);
							CloseHandle(hThread[1]);
							CloseHandle(hThread[2]);
							return 0x555; //"exit" signals an exit from the client side

						}

					} else if (menu == 2) 
					{
						char *ptr = ReadFromBufferString;
						while (*ptr) {
							*ptr = toupper((unsigned char)*ptr);
							ptr++;
						}

						move_res = check_valid_move(ReadFromBufferString);
						if (move_res == 1 )
						{
							strcpy(SendStr, "CLIENT_PLAYER_MOVE:");
							strcat(SendStr, ReadFromBufferString);
							strcat(SendStr, ";");
						}
						else
							printf("You selected Illigal Move, please insert your select again.\n");
					}
					else if (menu == 3)
					{
						if (STRINGS_ARE_EQUAL(ReadFromBufferString, "1"))
						{
							strcpy(SendStr, "CLIENT_REPLAY");
							error = 0;
						}
						else if (STRINGS_ARE_EQUAL(ReadFromBufferString, "2"))
						{
							strcpy(SendStr, "CLIENT_MAIN_MENU");
							error = 0;
						}
					}
					//sending the string to the server and updating the logfile
					if ((error != 1) && (strcmp(ReadFromBufferString, "\0") != 0))
					{
						if (((Client.PlayerMode == 0) && (send_new_string)) || (Client.PlayerMode) || (first_read == 0))
						{
							first_read = 1;
							if (send_new_string)
							{
								SendRes = SendString(SendStr, m_socket);
								send_new_string = 0;
							}
							if (SendRes == TRNS_FAILED)
							{
								printf("Socket error while trying to write data to socket\n");
								release_res = ReleaseMutex(mutex_buffer);
								if (release_res == FALSE) ReportErrorAndEndProgram();
								return 0x555;
							}
						}
					}
				}
			}
		}




		/* End Critical Section - finished reading the message. releases mutex*/

		release_res = ReleaseMutex(mutex_buffer);
		if (release_res == FALSE) ReportErrorAndEndProgram();

	}


	return 0;
}


static DWORD Message(void)				//the producer thread. recieves the client struct
{
	str InputString;
	DWORD wait_res;
	BOOL release_res;

	//checking the mutex for the writing option.
	while (ThreadEnd == 0)
	{
		wait_res = WaitForSingleObject(mutex_buffer, INFINITE); // waiting for input
		if (wait_res != WAIT_OBJECT_0) ReportErrorAndEndProgram();

		if (Client.CanWrite)
		{
			Client.CanWrite = 0;
			Client.CanRead = 1;

			if (Client.PlayerMode)
			{
				gets(InputString, sizeof(InputString)); //Reading a string from the keyboard
				strcpy(messages_to_server, InputString);
				send_new_string = 1;
			}

	/*		else
			{
				//wait for turn from client. need to understand how to do that.

				if (((first_write == 0) || ((Client.my_turn == 1) && start)) && !send_new_string)
				{
					if (!feof(Client.InputFile_ptr))
					{
						fgets(InputString, USERNAME_MAX_LENGTH, Client.InputFile_ptr);
						//printf("read from file \n");
						if (strchr(InputString, '\n') != NULL)
							InputString[strlen(InputString) - 1] = 0;
						strcpy(messages_to_server, InputString);
						send_new_string = 1;
					}
					else
					{
						release_res = ReleaseMutex(mutex_buffer);
						if (release_res == FALSE) ReportErrorAndEndProgram();
						return 0;
					}
				}
			}*/
			first_write = 1;
			//so far we have produced the message. now, adding the string to the messages buffer



			//releasing the mutex so the communication thread will be able to read from the buffer.
		}
		release_res = ReleaseMutex(mutex_buffer);
		if (release_res == FALSE) ReportErrorAndEndProgram();
	}
	return 0;
}


static void ReportErrorAndEndProgram()
{
	printf("Encountered error, ending program. Last Error = 0x%x Press any key to Exit\n", GetLastError());
	getchar();
	exit(1);
}


void MainClient(int port, char* server_ip)//FILE* logfile, FILE* inputfile, int port)
{
	SOCKADDR_IN clientService;
	BOOL release_res;
	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
					 //The WSADATA structure contains information about the Windows Sockets implementation.

					 //Call WSAStartup and check for errors.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");

	//Call the socket function and return its value to the m_socket variable. 
	// For this application, use the Internet address family, streaming sockets, and the TCP/IP protocol.

	// Create a socket.
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}
	/*
	The parameters passed to the socket function can be changed for different implementations.
	Error detection is a key part of successful networking code.
	If the socket call fails, it returns INVALID_SOCKET.
	The if statement in the previous code is used to catch any errors that may have occurred while creating
	the socket. WSAGetLastError returns an error number associated with the last error that occurred.
	*/


	//For a client to communicate on a network, it must connect to a server.
	// Connect to a server.

	//Create a sockaddr_in object clientService and set  values.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(server_ip); //Setting the IP address to connect to
	clientService.sin_port = htons(port); //Setting the port to connect to.

										  /*
										  AF_INET is the Internet address family.
										  */


										  // Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
										  // Check for general errors.
	while (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		printf("\nFailed connection to server on %s:%d Waiting 3 seconds and then we will try again.\n", server_ip, port);
		//WSACleanup();
		Sleep(3000);
		//return;
	}
	//else
	printf("\nConnected to server on %s:%d\n", server_ip, port);



	// Send and receive data.
	/*
	In this code, two integers are used to keep track of the number of bytes that are sent and received.
	The send and recv functions both return an integer value of the number of bytes sent or received,
	respectively, or an error. Each function also takes the same parameters:
	the active socket, a char buffer, the number of bytes to send or receive, and any flags to use.

	*/

	mutex_buffer = CreateMutex(NULL,
		TRUE,
		NULL
	);
	if (mutex_buffer == NULL) ReportErrorAndEndProgram();

	hThread[0] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)SendDataThread,
		NULL,
		0,
		NULL
	);
	hThread[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvDataThread,
		NULL,
		0,
		NULL
	);

	hThread[2] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)Message,
		NULL,
		0,
		NULL
	);

	release_res = ReleaseMutex(mutex_buffer);
	if (release_res == FALSE) ReportErrorAndEndProgram();

	WaitForMultipleObjects(3, hThread, TRUE, INFINITE);



	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	CloseHandle(hThread[2]);

	closesocket(m_socket);

	WSACleanup();

	return;
}
