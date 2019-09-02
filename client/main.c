#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define BUF_LEN 1024

int main(int argc, char *argv[]){
	char buffer[BUF_LEN + 1];
	struct sockaddr_in server_addr;
	int server_fd, n;

	char input_data[BUF_LEN];
	int len, msg_size;

	pid_t pid;
	
	char** dic_p = NULL;
	char** fil_p = NULL;
	char* ptr = NULL;
	char* ptr_p = NULL;

	int cont = 0;

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

//	printf("Test fork _ 1 %d \n",pid);

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
			memset(&buffer, 0x00, sizeof(buffer));
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

			// ---------- 명령어 처리 ------------ //

			if(strcmp(buffer,"ndic") == 0){
				free(dic_p);
				dic_p = NULL;
				continue;
			}

			if(strcmp(buffer,"nfil")== 0){
				free(fil_p);
				fil_p = NULL;
				continue;
			}

			// 디렉토리, 파일 별로 데이터가 들어옴 이걸 molloc을 이용하여 동적으로 관리할 계획
			if(strcmp(buffer,"dic") == 0){
				free(dic_p);
				dic_p = NULL;

				if((n = read(server_fd, buffer, BUF_LEN)) < 0){
					perror("read");
					exit(0);
				}
				else if(n == 0){
					puts("--- 서버 연결 종료(ERROR 1) ---");
					close(server_fd);
					kill(pid,SIGKILL);
					exit(0);
				}

				cont = 0;
				ptr_p = buffer;

				while(ptr != NULL){
					ptr = strchr(ptr + 1, '\n');
					cont++;
				}

				// 데이터 저장
				dic_p = (char **)malloc(sizeof(char) * cont + 1);
				cont = 0;

				while(ptr != NULL){
					ptr = strchr(ptr + 1, '\n');
					dic_p[cont] = (char*)malloc(sizeof(char) *20);
					memcpy(dic_p[cont],ptr_p,ptr - ptr_p);
					ptr_p = ptr + 1;
					cont++;
				}
				continue;
			}

			if(strcmp(buffer,"fil") == 0){				
				free(fil_p);
				fil_p = NULL;
				
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

				cont = 0;
				
				while(ptr != NULL){
					ptr = strchr(ptr + 1, '\n');
					cont++;
				}

				// 데이터 저장
				fil_p = (char **)malloc(sizeof(char) * cont + 1);
				cont = 0;
				ptr_p = buffer;

				while(ptr != NULL){
					ptr = strchr(ptr + 1, '\n');
					fil_p[cont] = (char*)malloc(sizeof(char) *20);
					memcpy(fil_p[cont],ptr_p,ptr - ptr_p - 1);
					ptr_p = ptr + 1;
					cont++;
				}
				continue;
			}

			if(strcmp(buffer,"lsfin") == 0){
				puts("(lsfin input)");
				int dic_s = sizeof(dic_p);
				int fil_s = sizeof(fil_p);

				for(int i = 0; i < dic_s; i++){
					puts(dic_p[i]);
				}

				for(int i =0; i < fil_s; i++){
					puts(fil_p[i]);
				}
				continue;
			}

			buffer[n] = '\0';
			puts(buffer);
		}
		
		// ----------- 명령어 끝 ---------------- //
	}
		

	return 0;
}

