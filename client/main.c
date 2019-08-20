#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define BUF_LEN 128

int main(int argc, char *argv[]){
	char buffer[BUF_LEN + 1];
	struct sockaddr_in server_addr;
	int server_fd, n;

	char input_data[BUF_LEN];
	int len, msg_size;

	pid_t pid;

	memset(&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;

	// 입력 인자 값으로 서버 설정
	if(argc <= 1){
		printf("---- 기본 서버 주소로 설정 됨 ----\n");
		server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		server_addr.sin_port = 9800;
	}
	else if(argc == 2){
		printf("---- 서버 주소 %s 설정 ----\n", argv[1]);
		server_addr.sin_addr.s_addr = inet_addr(argv[1]);
		server_addr.sin_port = 9800;

	}
	else if(argc == 3){
		printf("---- 서버 주소 %s : %s 설정 ----\n", argv[1],argv[2]);
		server_addr.sin_addr.s_addr = inet_addr(argv[1]);
		server_addr.sin_port = htons(atoi(argv[2]));
	}	
	else{
		printf("인자 값 오류 %s \n",argv[0]);
	}

	// 소켓 파일디스크립터 
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("sock");
		exit(0);
	}
	
	if(connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		perror("connect");
		exit(0);
	}

	// 로그인 기능 구현

	printf("id : ");
	scanf("%s", &buffer);
	
	if(write(server_fd,buffer,strlen(buffer)) < 0){
			perror("write");
			exit(0);
	}

	printf("pw : ");
	scanf("%s", &buffer);
	
	if(write(server_fd,buffer,strlen(buffer)) < 0){
			perror("write");
			exit(0);
	}
	memset(&buffer, 0x00, sizeof(buffer));
		
	//로그인 결과 
	if((n = read(server_fd, buffer, BUF_LEN)) < 0){
		perror("read");
		close(server_fd);
		exit(0);
	}
	
	puts(buffer);

	if(strcmp(buffer,"sss") == 0 ){	
		puts("로그인 성공");
	}
	else{
		puts("연결 실패");
		write(server_fd, "exit", 4);
		close(server_fd);
		exit(0);
	}

	// 프로세스 분기 
	pid = fork();

	printf("Test fork _ 1 %d \n",pid);

	if(pid == -1){
		perror("pid");
		exit(0);
	}

	if(pid == 0){ // 자식 프로세스

		while(1){
			fgets(input_data, BUF_LEN, stdin);
			input_data[strlen(input_data)- 1] = '\0';
				
			if(ferror(stdin)){
				perror("stdio");
			}

			if(write(server_fd,input_data,strlen(input_data)) < 0){
				perror("write");
				exit(0);
			}
		}
	}
	else{ // 부모 프로세스
			
		while(1){

			if((n = read(server_fd, buffer, BUF_LEN)) < 0){
				perror("read");
				exit(0);
			}
			else if(n == 0){
				puts("--- 서버 연결 종료 ---");
				close(server_fd);
				kill(pid,SIGKILL);
				exit(0);
			}
			buffer[n] = '\0';
			puts(buffer);
		}
	}
		

	return 0;
}

