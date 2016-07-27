#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#define PORT 8082

void errorHandling(char *message);
int main(int argc, char *argv[]) {
	int serv_sock;
	int clnt_sock;

	char buf[1024];

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	char status[] = "HTTP/1.0 200 OK/r/n";
	char header[] = "Server:A Simple Web Server\r\nContent-type: text/html\r\n\r\n";
	char body[] = "<html><head><title>A Simple Web Server</title></head><body><h2>Welcome!></h2><p>This is Beginner!</p></body></html>";

	serv_sock = socket(PF_INET, SOCK_STREAM,0);
	if (-1 == serv_sock) {
		errorHandling("socket() error");
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORT);	
	
	if (-1 == bind(serv_sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr))) {
		errorHandling("bind() error");	
	}

	if (-1 == listen(serv_sock, 5)) {
		errorHandling("listen() error");
	}
	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
	if (-1 == clnt_sock) {
		errorHandling("accept() error");
	}
   
	
	read(clnt_sock, buf, sizeof(buf)-1);
	printf("%s",buf);
	

	write(clnt_sock, status, sizeof(status));
	write(clnt_sock, header, sizeof(header));
	write(clnt_sock, body, sizeof(body));	
	
	close(clnt_sock);
	close(serv_sock);
	return 0;
} 

void errorHandling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);

}
