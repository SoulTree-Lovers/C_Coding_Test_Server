document.addEventListener("DOMContentLoaded", function() {
    document.getElementById('inputCode').value = localStorage.getItem('savedCode');
});

function sendData() {
    const inputValue = document.getElementById('inputCode').value;

    // 입력한 텍스트를 로컬 스토리지에 저장하여 나중에 유지합니다.
    localStorage.setItem('savedCode', inputValue);

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