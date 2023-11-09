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
        document.getElementById('result').textContent = `${data}`;
        localStorage.setItem('lastResult', data);
    })
    .catch(error => {
        console.error('오류:', error);
    });
}

// 페이지 로딩 시 마지막 결과를 표시
document.addEventListener("DOMContentLoaded", function() {
    const lastResult = localStorage.getItem('lastResult');
    if (lastResult) {
        document.getElementById('result').textContent = `${lastResult}`;
    }

    const savedCode = localStorage.getItem('savedCode');
    if (savedCode) {
        document.getElementById('inputCode').value = savedCode;
    }
});

function clearData() {
    document.getElementById('inputCode').value = '';
    document.getElementById('result').textContent = '';
    localStorage.removeItem('savedCode');
    localStorage.removeItem('lastResult');
}