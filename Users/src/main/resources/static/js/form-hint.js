const hint = document.getElementById("hint");
const button = document.getElementById("target");
const language = document.getElementById("language");
const code = document.getElementById("code");

const MAX_CODE_LENGTH = 16 * 1024;

function getByteLength(str) {
    return new TextEncoder().encode(str).length;
}

function updateHint() {
    const currentBytes = getByteLength(code.value);
    const isOverflow = currentBytes > MAX_CODE_LENGTH;

    let text = `最大支持 ${MAX_CODE_LENGTH} 字节，当前代码 ${currentBytes} 字节`;
    if (language.value === "java") {
        text = "使用 java 时主类需要是 Main！" + text;
    }

    hint.innerText = text;

    if (hint.classList.contains("opacity-0")) {
        hint.classList.remove("opacity-0");
        hint.classList.add("opacity-95");
    }
    if (isOverflow) {
        hint.classList.remove("text-gray-700");
        hint.classList.add("text-red-500");
    }
    else {
        hint.classList.remove("text-red-500");
        hint.classList.add("text-gray-700");
    }
}

window.onload = function () {
    button.addEventListener('mouseleave', function() {
        this.blur();
    });

    // 语言下拉框改变时刷新提示
    language.addEventListener('change', updateHint);

    // 代码输入实时刷新提示
    code.addEventListener('input', updateHint);

    // 初始化执行一次
    updateHint();
};