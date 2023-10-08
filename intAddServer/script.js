document.getElementById('submitButton').addEventListener('click', () => {
    const numberInput = document.getElementById('numberInput').value;
    console.log(numberInput);
    // 입력값이 빈 문자열인 경우 처리
    if (numberInput === '') {
        document.getElementById('result').innerText = '정수를 입력하세요.';
        return;
    }

    // 입력값이 정수로 변환 가능한지 확인
    const parsedInput = parseInt(numberInput);
    if (isNaN(parsedInput)) {
        document.getElementById('result').innerText = '올바른 정수를 입력하세요.';
        return;
    }

    // 서버에 정수 값을 전송
    fetch('http://localhost:8080', {
        method: 'POST',
        mode: 'cors', // CORS 요청 설정
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ inputNumber: parsedInput }),
    })
    .then(response => response.json())
    .then(data => {
        // 서버에서 반환된 결과를 화면에 출력
        document.getElementById('result').innerText = `서버에서 받은 값: ${data.result}`;
    })
    .catch(error => {
        console.error('오류:', error);
    });
});
