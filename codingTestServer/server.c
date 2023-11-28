#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <stddef.h>
#include <ctype.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int server_fd;

/* 민석 */
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

/* 민석 */
// 자식 프로세스의 실행시간을 제한하기 위한 함수
void handle_child_timeout(int sig) {
    if (sig == SIGALRM) {
        printf("자식 프로세스 타임아웃\n");
        exit(EXIT_FAILURE); // 자식 프로세스를 종료합니다
    }
}

/* 민석 */
// 프로세스를 생성하여 실행하는 함수
int run_process(const char* command, char* result, size_t result_size) {
    int status;
    int pipefd[2];      // 자식프로세스의 표준 출력을 저장하는 파이프
    int errpipefd[2];   // 자식프로세스의 표준 에러를 저장하는 파이프
    if(pipe(pipefd) == -1 || pipe(errpipefd)==-1) {
        perror("pipe");
        return 1;
    }
    
    pid_t pid;
    pid = fork(); // 새 프로세스 생성

    if (pid == -1) {
        // fork 실패
        strcpy(result, "프로세스 생성 실패.\n");
        return 1;
    } else if (pid == 0) {
        // 자식 프로세스
        // 표준 출력을 파이프의 쓰기 단락으로 재지정합니다.
        if(dup2(pipefd[1], STDOUT_FILENO) == -1){
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(pipefd[0]); // 읽기 단락을 닫습니다.
        close(pipefd[1]); // 쓰기 단락을 닫습니다.

        // 표준 출력을 파이프의 쓰기 단락으로 재지정합니다.
        if(dup2(errpipefd[1], STDERR_FILENO) == -1){
            perror("dup2_err");
            exit(EXIT_FAILURE);
        }
        close(errpipefd[0]);
        close(errpipefd[1]);

        // 자식 프로세스의 실행 시간 제한을 설정
        signal(SIGALRM, handle_child_timeout);
        alarm(3);

        // 외부 명령어를 실행
        execl("/bin/sh", "sh", "-c", command, (char*)NULL);
        // execl 실패 시 에러 발생, 프로세스 강제종료
        perror("execl");
        exit(EXIT_FAILURE);
    }else {
        // 부모 프로세스
        close(pipefd[1]); 
        close(errpipefd[1]);

        // 자식 프로세스의 종료를 대기합니다.
        waitpid(pid, &status, 0);

        // 자식 프로세스의 표준 에러 출력을 읽습니다.
        char errbuf[4096];
        ssize_t errcount = read(errpipefd[0], errbuf, sizeof(errbuf) - 1);
        if (errcount > 0) {
            errbuf[errcount] = '\0'; // NULL 문자로 종료
            // result 변수에 에러 메시지를 추가합니다.
            strncat(result, errbuf, result_size - strlen(result) - 1);
        }
        close(errpipefd[0]);

        // 자식 프로세스의 표준 출력을 읽습니다.
        ssize_t count = read(pipefd[0], result, result_size - 1); // count는 표준출력의 갯수
        if (count > 0) {    // 표준출력이 존재
            result[count] = '\0';
        } else if (count == 0 && errcount == 0) {   //표준출력이 존재하지 않음
            strcpy(result, "오답입니다.\n");
        }
        close(pipefd[0]);

        if (WIFSIGNALED(status)) { // 자식 프로세스가 신호에 의해 종료되었다면
            int signal_number = WTERMSIG(status);   // 종료시킨 신호의 번호를 반환
            if (signal_number == SIGALRM){  //  SIGALRM에 의해 종료되었다면
                strcpy(result, "실행 시간 초과입니다.\n");
                return -1;
            }
        } else if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            strcpy(result, "프로그램 실행 실패.\n");
            return -1;
        }
    }
    return WEXITSTATUS(status); // 자식프로세스의 종료 상태를 반환
}

/* 민석 */
// 사용자 코드 컴파일 함수
int compile_code(const char *filename, char *result,size_t result_size) {
    // 사용자의 코드를 컴파일
    char command[256];
    sprintf(command, "gcc %s -o output", filename);
    return run_process(command, result,sizeof(result));
}

/* 민석 */
// 문자열 끝의 공백문자를 제거하는 함수
// 테스트한 코드의 output을 정리해주는 역할
void rtrim(char *string){
    char *back = string + strlen(string);
    while(isspace(*--back));
    *(back+1) = '\0';
}

/* 민석 */
// 컴파일 된 코드를 실행하고 입력 예제를 테스트하여 출력 예제와 비교하는 함수
int run_tests(const char *command, const char *input_file, const char *expected_output_file, char* result) {
    FILE *fp_in = fopen(input_file, "r");
    FILE *fp_expected = fopen(expected_output_file, "r");
   
    if (fp_in == NULL || fp_expected == NULL) {
        sprintf(result, "입력 또는 예상 출력 파일을 열 수 없습니다.\n");
        if (fp_in) fclose(fp_in);
        if (fp_expected) fclose(fp_expected);
        return 1;
    }

    char input_line[256];
    char expected_line[256];
    char actual_output[BUFFER_SIZE]={};
    int fail_count = 0;
    int test_case_number = 1;
    int run_status;

    // 각 입력 케이스에 대해 테스트 실행
    while (fgets(input_line, sizeof(input_line), fp_in) != NULL && fgets(expected_line, sizeof(expected_line), fp_expected) != NULL) {
        // 임시 파일을 생성하여 입력을 저장
        char temp_input_file[] = "temp_input_XXXXXX"; // 무작위 이름의 임시파일을 생성
        int fd = mkstemp(temp_input_file);
        if (fd == -1) {
            sprintf(result, "임시 파일 생성 실패.\n");
            fclose(fp_in);
            fclose(fp_expected);
            return 1;
        }

        // 입력을 임시 파일에 쓰기
        write(fd, input_line, strlen(input_line));
        close(fd);

        // 명령어를 생성
        char test_command[1024];
        sprintf(test_command, "%s < %s", command, temp_input_file);
        // actual_output 배열 초기화
        memset(actual_output,0,sizeof(actual_output));
        // run_process를 사용하여 명령어 실행 및 결과 캡처
        run_status = run_process(test_command, actual_output,sizeof(actual_output));

        // 임시 파일 삭제
        unlink(temp_input_file);

        // run_process의 실행 결과 확인
        if (run_status != 0) {  // 프로세스의 실행이 비정상적인 종료 시
            strcpy(result, actual_output);
            fclose(fp_in);
            fclose(fp_expected);
            return 1;
        }

        // 예상출력과 실행결과에서의 불필요한 공백 제거
        rtrim(expected_line);
        rtrim(actual_output);

        // 결과와 예상 출력 비교
        if (strcmp(actual_output, expected_line) != 0) {
            sprintf(result + strlen(result), "\n테스트 케이스 %d 실패: 출력이 예상과 다릅니다.\n", test_case_number);
            sprintf(result + strlen(result), "기대했던 값: %s\n받은 값: %s\n", expected_line, actual_output);
            fail_count++;
        }
        test_case_number++;
    }

    // 모든 테스트 케이스가 성공적
    if(fail_count == 0){
        sprintf(result, "정답입니다!\n");
    }

    fclose(fp_in);
    fclose(fp_expected);

    return fail_count;
}

/* 민석 */
// 웹페이지의 정보를 가져오는 함수, 문제의 번호를 구분해줌
char* get_request_path(const char* request){
    static char path[256];
    if(sscanf(request, "POST %s ",path)==1){
        return path;
    }
    else if(sscanf(request, "GET %s ",path)==1){
        return path;
    }
    return NULL;
}

/* 승민 */
int main() {
    struct sockaddr_in address;
    socklen_t addr_size = sizeof(address);
    int opt = 1;

    // 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }
    // 소켓 옵션 설정: SO_REUSEADDR 설정
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("소켓 옵션 설정 실패");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
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
   
    while(1){
        // 클라이언트 연결 수락 및 데이터 처리
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addr_size)) < 0) {
        perror("연결 수락 실패");
            continue;
        }

        // 클라이언트로부터의 요청 읽기
        char buffer[BUFFER_SIZE];
        read(new_socket, buffer, sizeof(buffer));

        int problem_number=0;   
        char *path = get_request_path(buffer);

        if(strcmp(path,"/index.html")==0){
            problem_number = 1;
        }
        else if(strcmp(path,"/next_problem.html")==0){
            problem_number = 2;
        }

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
                    // printf("추출된 값: %s\n", extractedValue);
                    strcpy(buffer, extractedValue);
                }
            }
        }

        char test_input[256],test_expected_output[256], code_file[256];
        sprintf(test_input,"test_input%d.txt", problem_number);
        sprintf(test_expected_output,"test_expected_output%d.txt", problem_number);
        sprintf(code_file,"ccode%d.c", problem_number);
        saveCodeToCFile(code_file, buffer); // 코드 .c 파일로 저장
        
        char result[BUFFER_SIZE];

        // 코드 컴파일 및 실행 및 채점 결과 저장
        if (compile_code(code_file, result,sizeof(result)) == 0) { 
            if (run_tests("./output",test_input, test_expected_output, result) != 0) {
                printf("%s",result);
            }
            else{
                printf("테스트 케이스를 모두 통과하였습니다!\n");
            }
        } else {
            printf("컴파일 실패: %s\n", result);
        }

        // 클라이언트에게 채점 결과 전송
        char response[4096];
        sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/plain\nAccess-Control-Allow-Origin: *\n\n%s", result);
        send(new_socket, response, strlen(response), 0);
        printf("클라이언트에게 응답을 보냈습니다.\n");

        close(new_socket); // 소켓 닫기
    }

    close(server_fd);
    return 0;
}
