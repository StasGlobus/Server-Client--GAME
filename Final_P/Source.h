#ifndef SOCKET_EXAMPLE_SERVER_H
#define SOCKET_EXAMPLE_SERVER_H
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#define NUM_OF_WORKER_THREADS 2
#define MAX_LOOPS 3
#define SEND_STR_SIZE 100
#define WAIT_TIME 15
typedef struct {
	int ClientNumber;
	char username[SEND_STR_SIZE];
	char select[SEND_STR_SIZE];
}serv_client;

char* handle_new_user(char *name);
void MainServer(int portnum);
int game_res(char *user_0_sel, char *user_1_sel);
static void CleanupWorkerThreads();
char* rand_cpu_res();
static DWORD HelperThread();
void SendStringMult(char *sendstr);
static int FindFirstUnusedThreadSlot();
void w2f();
static DWORD ServiceThread(SOCKET *t_socket);
int users_want_to_play_cnt = 0;

HANDLE SharedMutex;


HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
//ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
__declspec(selectany) int user_num = 0;
__declspec(selectany) char current_name[100];
__declspec(selectany) int rdy = 2;
__declspec(selectany) int selected = 2;
__declspec(selectany) int connected_players = 0;
__declspec(selectany) int victor = 7;
__declspec(selectany) int p_turn = 1;
__declspec(selectany) int game_ended = 0;
__declspec(selectany) char p_name1[40];
__declspec(selectany) int ApprovedName = 1;
__declspec(selectany) char p_name2[40];
__declspec(selectany) int done = 5;
__declspec(selectany) int board[6][7];
__declspec(selectany) int turn = 1;
__declspec(selectany) int turnstotal = 0;
__declspec(selectany) char boardstr[44] = "000000000000000000000000000000000000000000";
__declspec(selectany) char* LogFile;
__declspec(selectany) serv_client clients_lst[2];

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/






#endif // SOCKET_EXAMPLE_SERVER_H