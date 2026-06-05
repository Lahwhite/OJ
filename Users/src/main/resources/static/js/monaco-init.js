require(['vs/editor/editor.main'], function () {
    const initialCode = document.getElementById('code').value || '';

    const editor = monaco.editor.create(document.getElementById('editor-container'), {
        value: initialCode,
        language: 'plaintext',    // 默认值，随后立刻被覆盖
        theme: 'vs',
        automaticLayout: true,
        minimap: { enabled: false },
        scrollBeyondLastLine: false,
        scrollbar: {
            vertical: 'auto',
            horizontal: 'auto'
        }
    });

    // 语言切换
    const languageSelect = document.getElementById('language');
    if (languageSelect) {
        // 语言映射
        const langMap = {
            'java': 'java',
            'cpp': 'cpp',
            'c': 'c',
            'python': 'python'
        };

        // 根据下拉框当前值设置编辑器语言
        function updateLanguage() {
            const selected = languageSelect.value;
            const monacoLang = langMap[selected] || 'plaintext';
            monaco.editor.setModelLanguage(editor.getModel(), monacoLang);
        }

        // 立即执行一次，让页面加载时就正确高亮
        updateLanguage();

        // 监听用户切换
        languageSelect.addEventListener('change', updateLanguage);
    }

    // 表单提交时同步内容
    const form = document.getElementById('form');
    if (form) {
        form.addEventListener('submit', function () {
            document.getElementById('code').value = editor.getValue();
        });
    }
});