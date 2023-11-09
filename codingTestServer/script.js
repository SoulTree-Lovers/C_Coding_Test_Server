function sendData() {
    const inputValue = document.getElementById('inputCode').value;
    console.log(inputValue);
    // 서버로 입력한 정수 보내기
    fetch('http://localhost:8080', {
        method: 'POST',
        body: inputValue
    })
    .then(response => response.text())
    .then(data => {
        document.getElementById('result').textContent = `서버에서 받은 값: ${data}`;
    })
    .catch(error => {
        console.error('오류:', error);
    });
}