#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include "fastcgi.c"
#define PORT 8084

void errorHandling(char *message);
void requestHandling(void *sock);

void sendData(void *sock, char *filename); // 向浏览器发送请求文件的内容
void catHTML(void *sock, char *filename); // 读取HTML文件内容
void catJPEG(void *sock, char *filename); // 读取图像文件内容
void sendError(void *sock);         // 请求出错响应
void catPHP(void *sock, char *filename, char  *query);


int main(int argc, char *argv[]) { int serv_sock;
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
	//clnt_addr_size = sizeof(clnt_addr);
	//clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
//	if (-1 == clnt_sock) {
//		errorHandling("accept() error");
//	}
   
	   
    while(1){
        // 接受客户端的请求
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
        if(-1 == clnt_sock){
            errorHandling("accept() error");
        }

        // 处理客户端请求
        requestHandling((void *) &clnt_sock);
    }

    // 关闭套接字
    close(serv_sock);

	
//	read(clnt_sock, buf, sizeof(buf)-1);
//	printf("%s",buf);
	

//	write(clnt_sock, status, sizeof(status));
//	write(clnt_sock, header, sizeof(header));
//	write(clnt_sock, body, sizeof(body));	
	
//	close(clnt_sock);
//	close(serv_sock);

} 

void errorHandling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);

}

void requestHandling(void *sock) {
	int clnt_sock = *((int *) sock);
	char buf[1024];
	char method[10];
	char filename[20];

	read(clnt_sock, buf, sizeof(buf)-1);

	if (NULL == strstr(buf, "HTTP/")) {
		sendError(sock);	
		close(clnt_sock);
		return ;
	}
	strcpy(method, strtok(buf, " /"));
	strcpy(filename, strtok(NULL, " /"));
	if (0 != strcmp(method, "GET")) {
		sendError(sock);
		close(clnt_sock);
		return ;
	}
	sendData(sock, filename);
}

void sendData(void *sock, char *filename) {
	int clnt_sock = *((int *) sock);
	char buf[20];
	char ext[10];
	strcpy(buf,filename);

	strtok(buf, ".");
	strcpy(ext, strtok(NULL, "."));
	if (0 == strcmp(ext, "php")) {
        catPHP(sock,filename,"");
	} else if (0 == strcmp(ext, "html")) {
		catHTML(sock, filename);
	} else if (0 == strcmp(ext, "jpg")) {
		catJPEG(sock,filename);
	} else {
		sendError(sock);
		close(clnt_sock);
		return ;
	}
}

void catHTML(void *sock, char *filename) {
	int clnt_sock = *((int *) sock);
	char buf[1024];
	FILE *fp;

	char status[] = "HTTP/1.0 200 OK\r\n";
	char header[] = "Server: A Simple Web Server\r\nContent-type: text/html\r\n\r\n";
	write(clnt_sock, status, strlen(status));
	write(clnt_sock, header, strlen(header));
	fp = fopen(filename,"r");
	if (NULL == fp) {
		sendError(sock);
		close(clnt_sock);
		errorHandling("open file failed!");
		return ;
	}

	fgets(buf, sizeof(buf), fp);
	while (!feof(fp)) {
		write(clnt_sock, buf, strlen(buf));
		fgets(buf, sizeof(buf), fp);
	}
	fclose(fp);
	close(clnt_sock);

}


void catJPEG(void *sock, char *filename) {
	int clnt_sock = *((int *)sock);
	char buf[1024];
	FILE *fp;
	FILE *fw;
	char status[] = "HTTP/1.0 200 OK \r\n";
	char header[] = "Server:A Simple Web Server\r\nContent-Type:image/jpeg\r\n\r\n";
	write(clnt_sock, status, strlen(status));
	write(clnt_sock, header, strlen(header));
	fp = fopen(filename, "rb");
	if (NULL == fp) {
		sendError(sock);
		close(clnt_sock);
		errorHandling("open file failed!");
		return ;
	}
	fw = fdopen(clnt_sock, "w");
	fread(buf, 1, sizeof(buf),fp);
	while (!feof(fp)) {
		fwrite(buf, 1, sizeof(buf), fw);
		fwrite(buf, 1, sizeof(buf), fp);
	}
	fclose(fw);
	fclose(fp);
	close(clnt_sock);
 
}



void sendError(void *sock){
    int clnt_sock = *((int *) sock);

    char status[] = "HTTP/1.0 400 Bad Request\r\n";
    char header[] = "Server: A Simple Web Server\r\nContent-Type: text/html\r\n\r\n";
    char body[] = "<html><head><title>Bad Request</title></head><body><p>请求出错，请检查！</p></body></html>";

    // 向客户端套接字发送信息
    write(clnt_sock, status, sizeof(status));
    write(clnt_sock, header, sizeof(header));
    write(clnt_sock, body, sizeof(body));
}


void catPHP(void *sockc, char *filename, char *query) {
	int clnt_sock = *((int*) sockc); 
	int sock;
	struct sockaddr_in serv_addr;
	int str_len;
	int contentLengthR;
	char msg[50];
	char buf[1024]; 
	char status[] = "HTTP/1.0 200 OK\r\n";
	char header[] = "Server:A Simple Web Server\r\n";
	sock = socket(PF_INET, SOCK_STREAM, 0); 
	if (-1 == sock) {
		errorHandling("sock error()");	
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(FCGI_HOST); 
	serv_addr.sin_port = htons(FCGI_PORT);
	if (-1 == connect(sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr))) {
		errorHandling("connect error");	
	}


	FCGI_BeginRequestRecord beginRecord;
	beginRecord.header = makeHeader(FCGI_BEGIN_REQUEST, FCGI_REQUEST_ID, sizeof(beginRecord.body), 0);
	beginRecord.body = makeBeginRequestBody(FCGI_RESPONDER);
	str_len = write(sock, &beginRecord, sizeof(beginRecord));
	char *params[][2] = {
		{"SCRIPT_FILENAME", "/home/vagrant/webserver/1.php"},
		{"REQUEST_METHOD", "GET"},
		{"QUERY_STRING", "name=ystop"},
		{"", ""}
	};
	int i,contentLength,paddingLength;
	FCGI_ParamsRecord *paramsRecordp;
	for (i = 0;params[i][0] != "";i++) {
		contentLength = strlen(params[i][0]) + strlen(params[i][1]) + 2;
		paddingLength = (contentLength % 8) == 0 ? 0 : 8 - (contentLength % 8);
		paramsRecordp = (FCGI_ParamsRecord *)malloc(sizeof(FCGI_ParamsRecord) + contentLength + paddingLength);
		paramsRecordp->nameLength = (unsigned char)strlen(params[i][0]);
		paramsRecordp->valueLength = (unsigned char)strlen(params[i][1]);
		paramsRecordp->header = makeHeader(FCGI_PARAMS, FCGI_REQUEST_ID, contentLength, paddingLength);
		memset(paramsRecordp->data, 0, contentLength+paddingLength);
		memcpy(paramsRecordp->data, params[i][0], strlen(params[i][0]));
		memcpy(paramsRecordp->data + strlen(params[i][0]), params[i][1], strlen(params[i][1]));
		str_len = write(sock, paramsRecordp, 8 + contentLength + paddingLength);
		if (-1 == str_len) {
			errorHandling("Write beginRecord failed!");	
		}
		printf("Write params %s %s\n",params[i][0], params[i][1]);
		free(paramsRecordp);
	}

    FCGI_Header stdinHeader;
    stdinHeader = makeHeader(FCGI_STDIN, FCGI_REQUEST_ID, 0 , 0);
    write(sock, &stdinHeader, sizeof(stdinHeader));
    FCGI_Header respHeader;
    char *message;
    str_len = read(sock, &respHeader, 8);
    if ( -1 == str_len) {
        errorHandling("read responder failed!");
    }
    
    if (respHeader.type == FCGI_STDOUT) {
        contentLengthR = 
            ((int)respHeader.contentLengthB1 << 8) + (int)respHeader.contentLengthB0;
        message = (char *)malloc(contentLengthR);
        read(sock, message, contentLengthR);
    }
    write(clnt_sock, status, strlen(status));
    write(clnt_sock, header, strlen(header));
    write(clnt_sock, message, contentLengthR);
    free(message);
    close(clnt_sock);
}

