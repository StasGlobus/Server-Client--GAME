#define _CRT_SECURE_NO_WARNINGS
#include "main.h"
int main(int argc, char* argv[])
{
	int port;
	char server_ip[16] = { '\0' };
	FILE* input_ptr;
	if (argc > 4) {
		printf("Too many arguments supplied.\n");
		return 0;
	}
	else if (argc < 4) {
		printf("Not Enogh arguments.\n");
		return 0;
	}
	port = atoi(argv[2]);
	memcpy(server_ip, argv[1], IPLENGTH);
	printf("\nIP:%s",server_ip);
	Client.CanRead = 0;
	Client.CanWrite = 1;
	Client.PlayerMode = 1; // Stas: return to this later
	strcpy(Client.username, argv[3]);
	MainClient(port,server_ip);

	return 0;

}