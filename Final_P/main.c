#define _CRT_SECURE_NO_WARNINGS
int main(int argc, char* argv[])
{
	int port;
	if (argc > 2) {
		printf("Too many arguments supplied.\n");
		return 0;
	}
	else if (argc < 2) {
		printf("Not Enogh arguments.\n");
		return 0;
	}
	port = atoi(argv[1]);
	printf("\nPort number is:%d\n", port);
	
	MainServer(port);//Client
		
	return 0;
	
}