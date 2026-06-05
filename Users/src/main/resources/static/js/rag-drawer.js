// rag-drawer.js - RAG问答抽屉面板交互逻辑
(function() {
    const drawer = document.getElementById('ragDrawer');
    const overlay = document.getElementById('drawerOverlay');
    const openBtn = document.getElementById('openDrawer');   // 原有按钮ID
    const closeBtn = document.getElementById('closeDrawerBtn');
    const questionInput = document.getElementById('questionInput');
    const submitBtn = document.getElementById('submitQueryBtn');
    const answerDiv = document.getElementById('answerArea');

    // 打开抽屉
    function openDrawer() {
        if (!drawer || !overlay) return;
        drawer.classList.remove('translate-x-full');
        drawer.classList.add('translate-x-0');
        overlay.classList.remove('opacity-0', 'invisible', 'pointer-events-none');
        overlay.classList.add('opacity-100', 'visible', 'pointer-events-auto');
        document.body.classList.add('no-scroll');
        document.addEventListener('keydown', escHandler);
    }

    // 关闭抽屉
    function closeDrawer() {
        if (!drawer || !overlay) return;
        drawer.classList.remove('translate-x-0');
        drawer.classList.add('translate-x-full');
        overlay.classList.remove('opacity-100', 'visible', 'pointer-events-auto');
        overlay.classList.add('opacity-0', 'invisible', 'pointer-events-none');
        document.body.classList.remove('no-scroll');
        document.removeEventListener('keydown', escHandler);
    }

    function escHandler(e) {
        if (e.key === 'Escape') closeDrawer();
    }

    // 绑定事件
    if (openBtn) openBtn.addEventListener('click', openDrawer);
    if (closeBtn) closeBtn.addEventListener('click', closeDrawer);
    if (overlay) overlay.addEventListener('click', closeDrawer);
    if (drawer) drawer.addEventListener('click', (e) => e.stopPropagation());

    // ----- RAG 查询逻辑 -----
    function escapeHtml(str) {
        if (!str) return '';
        return str.replace(/[&<>]/g, function(m) {
            if (m === '&') return '&amp;';
            if (m === '<') return '&lt;';
            if (m === '>') return '&gt;';
            return m;
        });
    }

    async function submitQuery() {
        const question = questionInput ? questionInput.value.trim() : '';
        if (!question) {
            if (answerDiv) answerDiv.innerHTML = '<div class="text-amber-600 bg-amber-50 p-3 rounded-lg">⚠️ 请输入问题</div>';
            return;
        }
        // 加载状态
        if (answerDiv) answerDiv.innerHTML = '<div class="flex items-center justify-center gap-2 py-4"><div class="w-5 h-5 border-2 border-indigo-200 border-t-indigo-600 rounded-full animate-spin"></div><span>检索知识库中...</span></div>';
        if (submitBtn) {
            submitBtn.disabled = true;
            submitBtn.innerHTML = '<svg class="animate-spin h-5 w-5" ...>...</svg><span>思考中...</span>';
        }
        try {
            const res = await fetch('/api/rag/query', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ question: question })
            });
            if (!res.ok) throw new Error(`HTTP ${res.status}`);
            const data = await res.json();
            const answer = data.answer || '未获得有效回答';
            if (answerDiv) {
                answerDiv.innerHTML = `
                    <div class="flex items-start gap-2 text-emerald-700 mb-2">
                        <svg class="h-5 w-5" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12.75L11.25 15 15 9.75M21 12c0 1.268-.63 2.39-1.593 3.068..."/></svg>
                        <span class="font-medium">智能助手回复</span>
                    </div>
                    <div class="whitespace-pre-wrap">${escapeHtml(answer)}</div>
                `;
            }
        } catch (err) {
            console.error(err);
            if (answerDiv) answerDiv.innerHTML = `<div class="text-red-600 bg-red-50 p-3 rounded-lg">❌ 查询失败：${escapeHtml(err.message)}</div>`;
        } finally {
            if (submitBtn) {
                submitBtn.disabled = false;
                submitBtn.innerHTML = '<svg class="h-5 w-5" ...>...</svg><span>提交查询</span>';
            }
        }
    }

    if (submitBtn) submitBtn.addEventListener('click', submitQuery);
    if (questionInput) {
        questionInput.addEventListener('keydown', (e) => {
            if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
                e.preventDefault();
                submitQuery();
            }
        });
    }

    // 初始确保抽屉关闭
    if (drawer) drawer.classList.add('translate-x-full');
    if (overlay) overlay.classList.add('opacity-0', 'invisible', 'pointer-events-none');
    document.body.classList.remove('no-scroll');
})();