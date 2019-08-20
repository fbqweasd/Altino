#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


#define BUF_LEN 128
#define PORT 9800

int main(int argc, char *argv[]){
	char buffer[BUF_LEN];
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;

	int len, msg_size;
	char temp[20];
	pid_t pid;

	// 소켓 파일디스크립터 
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("soket");
		exit(0);
	}
	

	memset(&server_addr, 0x00, sizeof(server_addr));
	// 입력 인자 값으로 서버 설정
	if(argc <= 1){
		printf("---- 기본 서버 주소로 설정 됨 ----\n");

		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = PORT;	
	}
	else if(argc == 2){
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(atoi(argv[1]));	
	}
	else{
		printf("인자 값 오류 %s \n",argv[0]);
	}

	if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		perror("bind");
		exit(0);
	}

	if(listen(server_fd, 5) <0){
		perror("listen");
		exit(0);
	}

	memset(buffer, 0x00, sizeof(buffer));

	pid = fork(); // 임시로 분기 해놓음 
				  // 아직은 클라이언트가 2개 밖에 몬 붙음..

	while(1){
		len = sizeof(client_addr);
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);

		if(client_fd < 0){
			perror("accept");
			exit(0);
		}

		inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp, sizeof(temp));
		printf("Server : %s client connected. \n", temp);
		
		// login code
		msg_size = read(client_fd, buffer, BUF_LEN);
		buffer[msg_size] = NULL;

		while(1){
			msg_size = read(client_fd, buffer, BUF_LEN);
			buffer[msg_size] = NULL;

			if(errno == EAGAIN){
				printf("data NULL");
			}
			if(msg_size < 0){
				perror("read");	
			}
			printf("%s : %s \n",temp, buffer);	
			
			if(strcmp(buffer,"exit") == 0){
				break;
			}
			
			write(client_fd, buffer, msg_size);
		}
		write(client_fd, "bye",3);

		close(client_fd);
		printf("Server : %s client closed. \n", temp);	
	}
	
	
	close(server_fd);
	return 0;
}
