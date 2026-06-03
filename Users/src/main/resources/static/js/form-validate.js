const form = document.getElementById("form");
const hint = document.getElementById("hint");
const button = document.getElementById("target");

const username = document.getElementById("username");
const password = document.getElementById("password");
const confirmPassword = document.getElementById("confirmPassword");

const validateRules = {
    usernameRule: /^[a-zA-Z][a-zA-Z0-9_]{2,31}$/,
    passwordRule: /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#$%&'()*+,\-.\/:;<=>?@\[\\\]^_`{|}~])\S{8,64}$/
};

function showHint(text) {
    hint.innerText = text;
    if (hint.classList.contains("opacity-0")) {
        hint.classList.remove("opacity-0");
        hint.classList.add("opacity-95");
    }
    if (hint.classList.contains("text-red-500")) {
        hint.classList.remove("text-red-500");
        hint.classList.add("text-gray-700");
    }
}

function hideHint() {
    if (hint.classList.contains("opacity-95")) {
        hint.classList.remove("opacity-95");
        hint.classList.add("opacity-0");
    }
}

function showErrorMessage(text) {
    hint.innerText = text;
    if (hint.classList.contains("opacity-0")) {
        hint.classList.remove("opacity-0");
        hint.classList.add("opacity-95");
    }
    if (hint.classList.contains("text-gray-700")) {
        hint.classList.remove("text-gray-700");
        hint.classList.add("text-red-500");
    }
}

function validateInputs() {
    const usernameString = username.value.trim();
    const passwordString = password.value.trim();
    if (confirmPassword !== null) {
        const confirmPasswordString = confirmPassword.value.trim();
        if (usernameString === "" || passwordString === "" || confirmPasswordString === "") {
            showErrorMessage("请勿输入空白字符");
            return false;
        }
        if (!validateRules.usernameRule.test(usernameString)) {
            showErrorMessage("用户名格式错误");
            return false;
        }
        if (!validateRules.passwordRule.test(passwordString)) {
            showErrorMessage("密码格式错误")
            return false;
        }
        if (passwordString !== confirmPasswordString) {
            showErrorMessage("两次密码不一致")
            return false;
        }
        return true;
    }
    if (usernameString === "" || passwordString === "") {
        showErrorMessage("请勿输入空白字符");
        return false;
    }
    return true;
}

window.onload = function () {
    button.addEventListener('mouseleave', function() {
        this.blur();
    });

    username.addEventListener("focus", function () {
        showHint("提示：仅支持字母、数字、下划线，首字符必须为字母，长度3-32位");
    });
    username.addEventListener("blur", function () {
        hideHint();
    });

    password.addEventListener("focus", function () {
        showHint("提示：需包含大小写、数字、特殊字符，长度8-64位（不支持空格）");
    });
    password.addEventListener("blur", function () {
        hideHint();
    });

    if (confirmPassword != null) {
        confirmPassword.addEventListener("focus", function () {
            showHint("提示：请保证两次输入密码一致");
        });
        confirmPassword.addEventListener("blur", function () {
            hideHint();
        });
    }

    form.addEventListener("submit", function (event) {
        if (!validateInputs()) {
            event.preventDefault();
        }
    })
};