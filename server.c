#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024

void handleRequest(int client_socket) {
    char buffer[MAX_BUFFER_SIZE];
    
    // 클라이언트로부터 데이터를 읽고 정수로 변환
    ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    if (received_bytes <= 0) {
        perror("데이터 수신 오류");
        return;
    }
    
    int inputNumber;
    if (sscanf(buffer, "%d", &inputNumber) != 1) {
        printf("inputNumber: %d\n", inputNumber);
        perror("정수로 변환 오류");
        return;
    }
    
    // 입력받은 정수에 10을 더한 값을 클라이언트로 전송
    int result = inputNumber + 10;
    snprintf(buffer, sizeof(buffer), "%d", result);
    send(client_socket, buffer, strlen(buffer), 0);
    
    // CORS 헤더 추가
    char corsHeader[] = "Access-Control-Allow-Origin: *"; // 모든 도메인에서 요청 허용
    send(client_socket, corsHeader, strlen(corsHeader), 0);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);
    
    // 서버 소켓 생성
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("소켓 생성 오류");
        exit(1);
    }
    
    // 서버 주소 설정
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);
    
    // 소켓과 서버 주소를 바인딩
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("바인딩 오류");
        exit(1);
    }
    
    // 서버가 클라이언트 연결 요청을 기다림
    if (listen(server_socket, 5) == -1) {
        perror("대기 오류");
        exit(1);
    }
    
    printf("서버가 %d 포트에서 대기 중입니다...\n", PORT);
    
    while (1) {
        // 클라이언트 연결 수락
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len)) == -1) {
            perror("연결 수락 오류");
            exit(1);
        }
        
        printf("클라이언트 연결이 수락되었습니다.\n");
        
        // 요청 처리
        handleRequest(client_socket);
        
        // 클라이언트 소켓 닫기
        close(client_socket);
        printf("클라이언트 연결이 종료되었습니다.\n");
    }
    
    // 서버 소켓 닫기
    close(server_socket);
    
    return 0;
}
