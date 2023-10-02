#include <stdio.h>
#include <stdlib.h>

// 채점 함수 정의
int grading(char user_answers[10], char correct_answers[10]) {
    int score = 0;
    for (int i = 0; i < 10; i++) {
        if (user_answers[i] == correct_answers[i]) {
            score++;
        }
    }
    return score;
}

int main() {
    // 정답을 저장할 배열
    char correct_answers[10];
    
    // 정답 파일에서 정답 읽기
    for (int i = 0; i < 10; i++) {
        char filename[20];
        sprintf(filename, "answer/answer%d.txt", i + 1); // 파일 이름 생성
        FILE *answer_file = fopen(filename, "r");
        if (answer_file == NULL) {
            printf("정답 파일을 열 수 없습니다.\n");
            return 1;
        }
        fscanf(answer_file, "%c", &correct_answers[i]);
        fclose(answer_file);
    }

    // 사용자 입력 받기
    char user_answers[10];
    for (int i = 0; i < 10; i++) {
        printf("문제 %d의 답을 입력하세요: ", i + 1);
        scanf(" %c", &user_answers[i]);
    }

    // 채점 함수 호출하여 결과 출력
    int score = grading(user_answers, correct_answers);
    printf("맞은 문제 수: %d / 10\n", score);

    return 0;
}
