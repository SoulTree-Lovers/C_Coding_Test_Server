async function sendData() {
    try {
        // const inputValue = document.getElementById('inputCode').value;
        const inputValue = await editor.getValue(); // CodeMirror 에디터에서 코드를 가져오도록 수정

        // 입력한 텍스트를 로컬 스토리지에 저장하여 나중에 유지합니다.
        localStorage.setItem('savedCode', inputValue);

        const currentPage = window.location.pathname.split('/').pop();
        const endpoint = currentPage === 'index.html' ? '/index.html' : '/next_problem.html'

        console.log(inputValue); // for debugging delete later

        // 서버로 입력한 정수 보내기
        const response = await fetch(`http://localhost:8080${endpoint}`, {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain;charset=UTF-8',
            },
            body: inputValue,
        });

        if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
        }

        const data = await response.text();

        document.getElementById('result').textContent = `${data}`;
        localStorage.setItem('lastResult', data);
    } catch (error) {
        console.error('오류:', error);
    }
}

// 페이지 로딩 시 마지막 결과를 표시
document.addEventListener("DOMContentLoaded", async function() {
    const lastResult = await localStorage.getItem('lastResult');
    if (lastResult) {
        document.getElementById('result').textContent = `${lastResult}`;
    }

    const savedCode = await localStorage.getItem('savedCode');
    if (savedCode) {
        document.getElementById('inputCode').value = savedCode;
        editor.setValue(savedCode);
    }
});

async function clearData() {
    editor.setValue("");
    document.getElementById('result').textContent = '';
    localStorage.removeItem('savedCode');
    localStorage.removeItem('lastResult');
}

function loadNextProblem() {
    localStorage.removeItem('lastResult');
    // 다음 문제 페이지로 이동
    window.location.href = "next_problem.html";
}

function loadPrevProblem() {
    localStorage.removeItem('lastResult');
    // 이전 문제 페이지로 이동
    window.location.href = "index.html";
}

// Initialize CodeMirror
var editor = CodeMirror.fromTextArea(document.getElementById('inputCode'), {
    mode: "text/x-csrc",
    lineNumbers: true,
    theme: "dracula"
});