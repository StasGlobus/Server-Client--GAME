#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include <winsock2.h>

#define SERVER_ADDRESS_STR "127.0.0.1"
#define IPLENGTH 16
#define WAIT_TIME 15000
#define SERVER_PORT 2345




#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )

#define USERNAME_MAX_LENGTH 30
#define MAX_MESSAGE_LINE 100
typedef char str[MAX_MESSAGE_LINE];
typedef char buffer[3 * MAX_MESSAGE_LINE];
typedef struct {

	int ClientNumber;
	str username;
	int my_turn;
	int PlayerMode;
	int CanWrite;
	int CanRead;
	int CanListen;

}client;


//globals

SOCKET m_socket;
buffer messages_to_server;
static HANDLE mutex_buffer;
static HANDLE read_buffer;
client Client;
int menu;
__declspec(selectany) int first_write = 0;
__declspec(selectany) int ThreadEnd = 0;
__declspec(selectany) int send_new_string = 0;
__declspec(selectany) int start = 0;
HANDLE hThread[3];
//function declaration

int luah[6][7];

static DWORD Message(void);
static DWORD SendDataThread(void);
static DWORD RecvDataThread(void);
static void ReportErrorAndEndProgram();

void MainClient(int port, char* server_ip);//FILE* logfile, FILE* inputfile, int port);