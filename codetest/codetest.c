#include <stdio.h>
#include <stdlib.h>

int code_input() {
    FILE *fp = fopen("ccode.c", "w");
    if (!fp) {
        perror("Failed to open the file for writing");
        return 1;
    }

    printf("Enter your code. Press Ctrl+D (or equivalent) to end input.\n");

    char buffer[1024]; // 입력 버퍼
    while (fgets(buffer, sizeof(buffer), stdin)) { // 키보드 입력 받기
        fputs(buffer, fp); // 파일에 입력 저장
    }

    fclose(fp);
    printf("Code saved to ccode.c\n");

    return 0;
}

int main() {
    if (code_input() != 0) {
        return 1; // 코드 입력에 실패했을 경우 종료
    }

    // 1. 사용자의 코드를 컴파일합니다.
    if (system("gcc ccode.c -o ccode") != 0) {
        fprintf(stderr, "Compilation failed.\n");
        return 1;
    }

    // 2. 테스트 케이스 실행 및 결과 확인
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
        printf("Test passed!\n");
    } else {
        printf("Test failed. Expected: %d, Got: %d\n", expected_sum, user_sum);
    }

    // 3. 자원 정리
    fclose(expected_output);
    fclose(user_output);

    return 0;
}

