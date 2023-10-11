#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


#define PORT 8080


int main() {
    int server_fd, new_socket;
    struct sockaddr_in address, new_address;
    socklen_t addr_size;

    int opt = 1;
    int addrlen = sizeof(address);

    // 소켓 생성
    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == 0) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));

    // 소켓 옵션 설정: SO_REUSEADDR 설정
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("소켓 옵션 설정 실패");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORT);

    // 소켓을 포트에 바인딩
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("바인딩 실패");
        exit(EXIT_FAILURE);
    }

    // 클라이언트 연결 대기
    if (listen(server_fd, 5) < 0) {
        perror("대기 실패");
        exit(EXIT_FAILURE);
    }

    printf("서버가 포트 %d에서 실행 중입니다...\n", PORT);

    addr_size = sizeof(new_address);

    // 클라이언트 연결 수락 및 데이터 처리
    if ((new_socket = accept(server_fd, (struct sockaddr *)&new_address, &addr_size)) < 0) {
        perror("연결 수락 실패");
        exit(EXIT_FAILURE);
    }

    // 클라이언트로부터의 요청 읽기
    char buffer[1024];
    printf("buffer: %s\n", buffer);
    read(new_socket, buffer, sizeof(buffer));
    printf("buffer: %s\n", buffer);
    
    

    // Content-Length (body) 값 추출
    const char* contentLengthHeader = "Content-Length: ";
    const char* contentLengthStart = strstr(buffer, contentLengthHeader);
    
    if (contentLengthStart) {
        contentLengthStart += strlen(contentLengthHeader);
        int contentLength = atoi(contentLengthStart);

        // 본문 데이터 추출
        if (contentLength > 0) {
            const char* bodyStart = strstr(buffer, "\n\r") + 3; // 본문 시작 위치 계산
            if (bodyStart) {
                char extractedValue[contentLength + 1];
                strncpy(extractedValue, bodyStart, contentLength);
                extractedValue[contentLength] = '\0';

                printf("추출된 값: %s\n", extractedValue);
                strcpy(buffer, extractedValue);
            }
        }
    }

    int receivedValue = atoi(buffer);
    printf("receivedValue: %d\n", receivedValue);
    int newValue = receivedValue + 10;
    printf("newValue: %d\n", newValue);

    char tempBuffer[1024];
    sprintf(tempBuffer, "%d", newValue);
    const char * value = tempBuffer;

    strcpy(buffer, value);

    // CORS 설정을 포함한 응답 보내기
    char response[1024];
    sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/plain\nAccess-Control-Allow-Origin: *\n\n%d", newValue);
    send(new_socket, response, strlen(response), 0);
    printf("클라이언트에게 응답을 보냈습니다.\n");

    return 0;
}
