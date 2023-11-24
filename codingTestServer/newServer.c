// compile: gcc -o newServer newServer.c -pthread
// run: ./newServer
// 클라이언트로부터 요청을 받고 응답을 보낸 뒤에도 연결을 유지하는 서버 버전

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080

// 문자열을 .c 파일에 저장하는 함수
void saveCodeToCFile(const char *filename, const char *code) {
    FILE *file_pointer;

    // 파일을 쓰기 모드("w")로 열기
    file_pointer = fopen(filename, "w+");

    // 파일이 성공적으로 열렸는지 확인
    if (file_pointer == NULL) {
        printf("파일을 열 수 없습니다.\n");
        return; // 에러 발생 시 함수 종료
    }

    // 문자열을 파일에 쓰기
    fprintf(file_pointer, "%s", code);

    // 파일 닫기
    fclose(file_pointer);

    printf("문자열을 파일에 성공적으로 저장했습니다.\n");
}

// 사용자 코드 컴파일 함수
int compile_code() {
    // 사용자의 코드를 컴파일합니다.
    if (system("gcc ccode.c -o ccode") != 0) {
        fprintf(stderr, "Compilation failed.\n");
        return 1;
    }
    return 0;
}

// 테스트 케이스 실행 및 결과 확인 함수
int run_tests(char* result) {
    // 리다이렉션을 사용하여 입력과 출력을 파일로부터 가져오고 쓰기
    if (system("./ccode < test_input.txt > user_output.txt") != 0) {
        fprintf(stderr, "Runtime error.\n");
        return 1;
    }

    FILE *expected_output = fopen("test_expected_output.txt", "r");
    FILE *user_output = fopen("user_output.txt", "r");

    if (!expected_output || !user_output) {
        perror("Failed to open output files");
        return 1;
    }

    int expected_sum, user_sum;

    // 예제로 단 하나의 테스트 케이스만 확인합니다.
    fscanf(expected_output, "%d", &expected_sum);
    fscanf(user_output, "%d", &user_sum);

    if (expected_sum == user_sum) {
        sprintf(result, "정답입니다!\n");
    } else {
        sprintf(result, "실패입니다.. Expected: %d, Got: %d\n", expected_sum, user_sum);
    }

    // 자원 정리
    fclose(expected_output);
    fclose(user_output);

    return 0;
}

// Function to handle each client connection
void *handle_connection(void *socket_desc) {
    int new_socket = *(int *)socket_desc;

    // 클라이언트로부터의 요청 읽기
    char buffer[4096];
    printf("buffer: %s\n", buffer);
    read(new_socket, buffer, sizeof(buffer));
    printf("buffer: %s\n", buffer);

    // Content-Length (body) 값 추출
    const char *contentLengthHeader = "Content-Length: ";
    const char *contentLengthStart = strstr(buffer, contentLengthHeader);

    if (contentLengthStart) {
        contentLengthStart += strlen(contentLengthHeader);
        int contentLength = atoi(contentLengthStart);

        // 본문 데이터 추출
        if (contentLength > 0) {
            const char *bodyStart = strstr(buffer, "\n\r") + 3; // 본문 시작 위치 계산
            if (bodyStart) {
                char extractedValue[contentLength + 1];
                strncpy(extractedValue, bodyStart, contentLength);
                extractedValue[contentLength] = '\0';

                printf("추출된 값: %s\n", extractedValue);
                strcpy(buffer, extractedValue);
            }
        }
    }

    // 저장된 사용자 코드를 파일에 저장
    saveCodeToCFile("ccode.c", buffer);

    char result[1024] = ""; // 채점 결과 저장

    if (compile_code() == 0) {
        if (run_tests(result) != 0) {
            sprintf(result, "실패입니다.\n");
        }
    } else {
        sprintf(result, "컴파일 실패입니다.\n");
    }

    // Send the response back to the client
    char response[1024];
    sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/plain\nAccess-Control-Allow-Origin: *\n\n%s", result);
    send(new_socket, response, strlen(response), 0);
    printf("클라이언트에게 응답을 보냈습니다.\n");

    // Close the socket for this client
    close(new_socket);

    // Free the socket descriptor
    free(socket_desc);

    return NULL;
}

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

    while (1) {
        addr_size = sizeof(new_address);

        // 클라이언트 연결 수락
        if ((new_socket = accept(server_fd, (struct sockaddr *)&new_address, &addr_size)) < 0) {
            perror("연결 수락 실패");
            exit(EXIT_FAILURE);
        }

        // Create a new thread to handle the connection
        pthread_t thread_id;
        int *new_socket_ptr = malloc(sizeof(int));
        *new_socket_ptr = new_socket;

        if (pthread_create(&thread_id, NULL, handle_connection, (void *)new_socket_ptr) < 0) {
            perror("스레드 생성 실패");
            exit(EXIT_FAILURE);
        }

        // Detach the thread to allow it to run independently
        pthread_detach(thread_id);
    }
    return 0;
}