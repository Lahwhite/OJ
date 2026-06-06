// 题目模块前端脚本：负责页面状态、接口调用与交互行为
const API_BASE = new URL("./v1/problems", window.location.href).pathname.replace(/\/$/, "");
const STATUS_API_BASE = new URL("./v1/problem-status", window.location.href).pathname.replace(/\/$/, "");
const TOKEN_STORAGE_KEY = "oj-problems-admin-token";

// 页面状态集中保存在这里，方便统一读写
const state = {
    problems: [],
    selectedProblemId: null,
    selectedProblem: null,
    page: 1,
    size: 20,
    total: 0,
    difficulty: "",
    tag: "",
    keyword: "",
    token: window.localStorage.getItem(TOKEN_STORAGE_KEY) || "",
    currentUserId: null,
    problemStatuses: new Map(),
};

// 缓存 DOM 节点，减少重复查询带来的开销
const els = {
    notice: document.querySelector("#notice"),
    homeLink: document.querySelector("#homeLink"),
    refreshButton: document.querySelector("#refreshButton"),
    filterForm: document.querySelector("#filterForm"),
    resetFilterButton: document.querySelector("#resetFilterButton"),
    keywordFilter: document.querySelector("#keywordFilter"),
    difficultyFilter: document.querySelector("#difficultyFilter"),
    tagFilter: document.querySelector("#tagFilter"),
    adminToken: document.querySelector("#adminToken"),
    adminStatus: document.querySelector("#adminStatus"),
    saveTokenButton: document.querySelector("#saveTokenButton"),
    clearTokenButton: document.querySelector("#clearTokenButton"),
    createProblemButton: document.querySelector("#createProblemButton"),
    pageSize: document.querySelector("#pageSize"),
    problemCount: document.querySelector("#problemCount"),
    problemList: document.querySelector("#problemList"),
    pageLabel: document.querySelector("#pageLabel"),
    previousPageButton: document.querySelector("#previousPageButton"),
    nextPageButton: document.querySelector("#nextPageButton"),
    emptyDetail: document.querySelector("#emptyDetail"),
    problemDetail: document.querySelector("#problemDetail"),
    editorModal: document.querySelector("#editorModal"),
    editorTitle: document.querySelector("#editorTitle"),
    editorProblemId: document.querySelector("#editorProblemId"),
    closeEditorButton: document.querySelector("#closeEditorButton"),
    cancelEditorButton: document.querySelector("#cancelEditorButton"),
    problemForm: document.querySelector("#problemForm"),
    titleInput: document.querySelector("#titleInput"),
    difficultyInput: document.querySelector("#difficultyInput"),
    tagsInput: document.querySelector("#tagsInput"),
    timeLimitInput: document.querySelector("#timeLimitInput"),
    memoryLimitInput: document.querySelector("#memoryLimitInput"),
    descriptionInput: document.querySelector("#descriptionInput"),
    inputDescriptionInput: document.querySelector("#inputDescriptionInput"),
    outputDescriptionInput: document.querySelector("#outputDescriptionInput"),
    sampleInputInput: document.querySelector("#sampleInputInput"),
    sampleOutputInput: document.querySelector("#sampleOutputInput"),
    publicInput: document.querySelector("#publicInput"),
    addTestCaseButton: document.querySelector("#addTestCaseButton"),
    testCaseEditorList: document.querySelector("#testCaseEditorList"),
};

// 页面辅助函数：拆分复杂交互，便于维护
function initFromQuery() {
    const params = new URLSearchParams(window.location.search);
    const returnUrl = params.get("return_url");
    // 条件分支：根据当前页面状态做不同处理
    if (returnUrl && isAllowedReturnUrl(returnUrl)) {
        els.homeLink.href = returnUrl;
    }
    const userId = Number(params.get("user_id") || params.get("userId"));
    // 条件分支：根据当前页面状态做不同处理
    if (Number.isInteger(userId) && userId > 0) {
        state.currentUserId = userId;
    }
    els.adminToken.value = state.token;
    updateAdminStatus();
}

// 页面辅助函数：拆分复杂交互，便于维护
function isAllowedReturnUrl(value) {
    try {
        const url = new URL(value, window.location.href);
        // 返回计算结果或提前结束当前流程
        return url.protocol === "http:" || url.protocol === "https:";
    } catch {
        // 返回计算结果或提前结束当前流程
        return false;
    }
}

// 页面辅助函数：拆分复杂交互，便于维护
function detailUrl(problemId) {
    const params = new URLSearchParams({ id: String(problemId) });
    const returnUrl = new URLSearchParams(window.location.search).get("return_url");
    // 条件分支：根据当前页面状态做不同处理
    if (returnUrl && isAllowedReturnUrl(returnUrl)) params.set("return_url", returnUrl);
    // 条件分支：根据当前页面状态做不同处理
    if (state.currentUserId) params.set("user_id", String(state.currentUserId));
    // 返回计算结果或提前结束当前流程
    return `./problem-detail.html?${params.toString()}`;
}

// 页面辅助函数：拆分复杂交互，便于维护
function setNotice(message, type = "info") {
    // 条件分支：根据当前页面状态做不同处理
    if (!message) {
        els.notice.className = "mb-5 hidden rounded-xl border px-4 py-3 text-sm";
        els.notice.textContent = "";
        return;
    }
    const themes = {
        info: "border-blue-200 bg-blue-50 text-blue-700",
        success: "border-green-200 bg-green-50 text-green-700",
        error: "border-red-200 bg-red-50 text-red-700",
    };
    els.notice.className = `mb-5 rounded-xl border px-4 py-3 text-sm ${themes[type] || themes.info}`;
    els.notice.textContent = message;
}

// 页面辅助函数：拆分复杂交互，便于维护
function updateAdminStatus() {
    const enabled = Boolean(state.token);
    els.adminStatus.textContent = enabled ? "已配置" : "未配置";
    els.adminStatus.className = enabled
        ? "rounded-lg bg-green-50 px-3 py-1 text-xs font-medium text-green-700"
        : "rounded-lg bg-gray-100 px-3 py-1 text-xs font-medium text-gray-500";
}

// 异步流程：通常负责接口请求与界面回填
async function requestJson(url, options = {}) {
    const response = await fetch(url, {
        ...options,
        headers: {
            "Content-Type": "application/json",
            ...(options.headers || {}),
        },
    });
    const text = await response.text();
    const body = text ? JSON.parse(text) : null;
    // 条件分支：根据当前页面状态做不同处理
    if (!response.ok || (body && body.code >= 400)) {
        throw new Error(body?.message || `请求失败：${response.status}`);
    }
    // 返回计算结果或提前结束当前流程
    return body?.data ?? null;
}

// 页面辅助函数：拆分复杂交互，便于维护
function authHeaders() {
    // 条件分支：根据当前页面状态做不同处理
    if (!state.token) {
        throw new Error("请先保存管理员 JWT。");
    }
    // 返回计算结果或提前结束当前流程
    return { Authorization: `Bearer ${state.token}` };
}

// 页面辅助函数：拆分复杂交互，便于维护
function listUrl() {
    const params = new URLSearchParams({
        page: String(state.page),
        size: String(state.size),
    });
    // 条件分支：根据当前页面状态做不同处理
    if (state.difficulty) params.set("difficulty", state.difficulty);
    // 条件分支：根据当前页面状态做不同处理
    if (state.tag) params.set("tag", state.tag);
    // 条件分支：根据当前页面状态做不同处理
    if (state.keyword) params.set("keyword", state.keyword);
    // 返回计算结果或提前结束当前流程
    return `${API_BASE}?${params.toString()}`;
}

// 异步流程：通常负责接口请求与界面回填
async function loadProblems({ keepSelection = false } = {}) {
    setNotice("");
    els.problemList.innerHTML = loadingHtml("正在加载题目...");
    const pageData = await requestJson(listUrl());
    state.problems = pageData.problems || [];
    state.total = pageData.total || 0;
    state.page = pageData.page || state.page;
    state.size = pageData.size || state.size;
    await loadProblemStatuses();

    // 条件分支：根据当前页面状态做不同处理
    if (!keepSelection || !state.problems.some((problem) => problem.id === state.selectedProblemId)) {
        state.selectedProblemId = null;
        state.selectedProblem = null;
    }
    renderProblems();
    renderPagination();

    // 条件分支：根据当前页面状态做不同处理
    if (!state.selectedProblemId && state.problems.length > 0) {
        await selectProblem(state.problems[0].id);
    } else if (state.selectedProblemId) {
        markActiveProblem();
    } else {
        showEmptyDetail();
    }
}

// 页面辅助函数：拆分复杂交互，便于维护
function loadingHtml(text) {
    // 返回计算结果或提前结束当前流程
    return `<div class="rounded-xl border border-gray-200 bg-gray-50 px-4 py-8 text-center text-sm text-gray-400">${text}</div>`;
}

// 页面辅助函数：拆分复杂交互，便于维护
function emptyHtml(text) {
    // 返回计算结果或提前结束当前流程
    return `<div class="rounded-xl border border-dashed border-gray-300 px-4 py-8 text-center text-sm text-gray-400">${text}</div>`;
}

// 异步流程：通常负责接口请求与界面回填
async function loadProblemStatuses() {
    state.problemStatuses = new Map();
    // 条件分支：根据当前页面状态做不同处理
    if (!state.currentUserId) {
        return;
    }
    try {
        const data = await requestJson(`${STATUS_API_BASE}/users/${state.currentUserId}`);
        // 遍历集合，为每个元素绑定逻辑或生成视图
        (data?.statuses || []).forEach((status) => {
            state.problemStatuses.set(Number(status.problemId), status);
        });
    } catch (error) {
        setNotice("个人 AC 状态暂时不可用，仍可继续浏览题目。", "info");
    }
}

// 页面辅助函数：拆分复杂交互，便于维护
function renderProblems() {
    els.problemCount.textContent = `共 ${state.total} 道题目`;
    // 条件分支：根据当前页面状态做不同处理
    if (state.problems.length === 0) {
        els.problemList.innerHTML = emptyHtml("没有找到符合条件的题目。");
        return;
    }
    els.problemList.innerHTML = state.problems.map((problem) => `
        <button type="button" class="problem-card w-full bg-white text-left ${problem.id === state.selectedProblemId ? "is-active" : ""}" data-problem-id="${problem.id}">
            <div class="grid grid-cols-[28px_minmax(0,1fr)] items-start gap-3">
                ${statusIconHtml(problem.id)}
                <div class="min-w-0">
                    <div class="flex items-center justify-between gap-3">
                        <span class="text-xs font-medium text-gray-400">P${problem.id}</span>
                        ${difficultyBadge(problem.difficulty)}
                    </div>
                    <h3 class="mt-3 line-clamp-2 font-bold text-gray-800">${escapeHtml(problem.title)}</h3>
                    <div class="mt-3 flex flex-wrap gap-2">${tagBadges(problem.tags)}</div>
                    <div class="mt-3 flex items-center justify-between gap-3 text-xs text-gray-400">
                        <span>通过率 ${formatRate(problem.passRate)}</span>
                        <span>${problem.acceptedCount || 0} / ${problem.submissionCount || 0}</span>
                    </div>
                </div>
            </div>
        </button>
    `).join("");
    // 遍历集合，为每个元素绑定逻辑或生成视图
    els.problemList.querySelectorAll("[data-problem-id]").forEach((button) => {
        button.addEventListener("click", () => selectProblem(Number(button.dataset.problemId)));
    });
}

// 页面辅助函数：拆分复杂交互，便于维护
function renderPagination() {
    const totalPages = Math.max(1, Math.ceil(state.total / state.size));
    els.pageLabel.textContent = `第 ${state.page} / ${totalPages} 页`;
    els.previousPageButton.disabled = state.page <= 1;
    els.nextPageButton.disabled = state.page >= totalPages;
}

// 页面辅助函数：拆分复杂交互，便于维护
function difficultyBadge(difficulty) {
    const themes = {
        easy: ["简单", "bg-green-50 text-green-700"],
        medium: ["中等", "bg-yellow-50 text-yellow-700"],
        hard: ["困难", "bg-red-50 text-red-700"],
    };
    const [label, classes] = themes[difficulty] || [difficulty || "未知", "bg-gray-100 text-gray-600"];
    // 返回计算结果或提前结束当前流程
    return `<span class="rounded-lg px-3 py-1 text-xs font-medium ${classes}">${escapeHtml(label)}</span>`;
}

// 页面辅助函数：拆分复杂交互，便于维护
function tagBadges(tags = []) {
    // 条件分支：根据当前页面状态做不同处理
    if (!tags.length) return '<span class="text-xs text-gray-400">暂无标签</span>';
    // 返回计算结果或提前结束当前流程
    return tags.map((tag) => `<span class="rounded-lg bg-blue-50 px-2 py-1 text-xs font-medium text-blue-600">${escapeHtml(tag)}</span>`).join("");
}

// 页面辅助函数：拆分复杂交互，便于维护
function getProblemStatus(problemId) {
    // 返回计算结果或提前结束当前流程
    return state.problemStatuses.get(Number(problemId)) || null;
}

// 页面辅助函数：拆分复杂交互，便于维护
function statusIconHtml(problemId) {
    // 条件分支：根据当前页面状态做不同处理
    if (!state.currentUserId) {
        // 返回计算结果或提前结束当前流程
        return '<span class="status-mark status-unknown" aria-label="未选择用户">-</span>';
    }
    const status = getProblemStatus(problemId);
    // 条件分支：根据当前页面状态做不同处理
    if (status?.accepted) {
        // 返回计算结果或提前结束当前流程
        return '<span class="status-mark status-accepted" title="已通过" aria-label="已通过">✓</span>';
    }
    // 返回计算结果或提前结束当前流程
    return '<span class="status-mark status-unaccepted" title="未通过" aria-label="未通过">-</span>';
}

// 页面辅助函数：拆分复杂交互，便于维护
function statusSummaryHtml(problemId) {
    // 条件分支：根据当前页面状态做不同处理
    if (!state.currentUserId) {
        // 返回计算结果或提前结束当前流程
        return "";
    }
    const status = getProblemStatus(problemId);
    const accepted = Boolean(status?.accepted);
    // 返回计算结果或提前结束当前流程
    return `
        <section class="mt-5 rounded-xl border ${accepted ? "border-green-100 bg-green-50" : "border-gray-200 bg-gray-50"} p-4">
            <div class="flex flex-wrap items-center justify-between gap-3">
                <div class="flex items-center gap-3">
                    ${statusIconHtml(problemId)}
                    <div>
                        <p class="text-sm font-medium ${accepted ? "text-green-700" : "text-gray-600"}">${accepted ? "已通过" : "未通过"}</p>
                        <p class="mt-1 text-xs text-gray-400">用户 #${state.currentUserId} 的题目状态</p>
                    </div>
                </div>
                <div class="grid gap-3 text-sm sm:grid-cols-3">
                    <div><span class="text-gray-400">历史最高分</span><strong class="ml-2 text-gray-700">${status?.bestScore ?? 0}</strong></div>
                    <div><span class="text-gray-400">最近得分</span><strong class="ml-2 text-gray-700">${status?.lastScore ?? "-"}</strong></div>
                    <div><span class="text-gray-400">最近提交</span><strong class="ml-2 text-gray-700">${formatDateTime(status?.lastSubmittedAt)}</strong></div>
                </div>
            </div>
        </section>
    `;
}

// 页面辅助函数：拆分复杂交互，便于维护
function markActiveProblem() {
    // 遍历集合，为每个元素绑定逻辑或生成视图
    els.problemList.querySelectorAll("[data-problem-id]").forEach((button) => {
        button.classList.toggle("is-active", Number(button.dataset.problemId) === state.selectedProblemId);
    });
}

// 异步流程：通常负责接口请求与界面回填
async function selectProblem(problemId) {
    state.selectedProblemId = problemId;
    markActiveProblem();
    els.emptyDetail.classList.add("hidden");
    els.problemDetail.classList.remove("hidden");
    els.problemDetail.innerHTML = loadingHtml("正在加载题目详情...");
    const problem = await requestJson(`${API_BASE}/${problemId}`);
    state.selectedProblem = problem;
    renderDetail(problem);
}

// 页面辅助函数：拆分复杂交互，便于维护
function showEmptyDetail() {
    els.problemDetail.classList.add("hidden");
    els.problemDetail.innerHTML = "";
    els.emptyDetail.classList.remove("hidden");
}

// 页面辅助函数：拆分复杂交互，便于维护
function renderDetail(problem) {
    els.emptyDetail.classList.add("hidden");
    els.problemDetail.classList.remove("hidden");
    els.problemDetail.innerHTML = `
        <div class="flex flex-wrap items-start justify-between gap-4">
            <div>
                <div class="flex flex-wrap items-center gap-2">
                    <span class="text-sm font-medium text-gray-400">#${problem.id}</span>
                    ${statusIconHtml(problem.id)}
                    ${difficultyBadge(problem.difficulty)}
                    ${problem.isPublic === false ? '<span class="rounded-lg bg-gray-100 px-3 py-1 text-xs font-medium text-gray-600">未公开</span>' : ""}
                </div>
                <h2 class="mt-3 text-2xl font-bold text-gray-800">${escapeHtml(problem.title)}</h2>
                <div class="mt-3 flex flex-wrap gap-2">${tagBadges(problem.tags)}</div>
            </div>
            <div class="flex flex-wrap gap-2">
                <a href="${detailUrl(problem.id)}" class="rounded-xl bg-green-600 px-4 py-3 text-sm font-medium text-white shadow-lg shadow-green-600/30 transition hover:bg-green-700">做题</a>
                <button id="editProblemButton" type="button" class="rounded-xl bg-indigo-600 px-4 py-3 text-sm font-medium text-white transition hover:bg-indigo-700">编辑</button>
                <button id="deleteProblemButton" type="button" class="rounded-xl bg-red-50 px-4 py-3 text-sm font-medium text-red-600 transition hover:bg-red-100">删除</button>
            </div>
        </div>

        <dl class="mt-5 grid gap-3 rounded-xl bg-gray-50 p-4 text-sm sm:grid-cols-3">
            <div><dt class="text-gray-400">时间限制</dt><dd class="mt-1 font-medium text-gray-700">${problem.timeLimit} ms</dd></div>
            <div><dt class="text-gray-400">内存限制</dt><dd class="mt-1 font-medium text-gray-700">${problem.memoryLimit} MB</dd></div>
            <div><dt class="text-gray-400">通过率</dt><dd class="mt-1 font-medium text-gray-700">${formatRate(problem.passRate)} (${problem.acceptedCount || 0} / ${problem.submissionCount || 0})</dd></div>
        </dl>

        ${statusSummaryHtml(problem.id)}
        ${detailSection("题目描述", textBlock(problem.description))}
        ${detailSection("输入说明", textBlock(problem.inputDescription))}
        ${detailSection("输出说明", textBlock(problem.outputDescription))}
        <section class="mt-6 grid gap-4 md:grid-cols-2">
            ${codeCard("样例输入", problem.sampleInput)}
            ${codeCard("样例输出", problem.sampleOutput)}
        </section>
        ${renderTestCases(problem.testCases || [])}
    `;
    document.querySelector("#editProblemButton").addEventListener("click", () => openEditor(problem));
    document.querySelector("#deleteProblemButton").addEventListener("click", () => deleteProblem(problem));
}

// 页面辅助函数：拆分复杂交互，便于维护
function detailSection(title, content) {
    // 返回计算结果或提前结束当前流程
    return `<section class="mt-6"><h3 class="text-lg font-bold text-gray-800">${title}</h3>${content}</section>`;
}

// 页面辅助函数：拆分复杂交互，便于维护
function textBlock(text) {
    // 返回计算结果或提前结束当前流程
    return `<p class="mt-3 whitespace-pre-wrap break-words leading-7 text-gray-700">${escapeHtml(text || "")}</p>`;
}

// 页面辅助函数：拆分复杂交互，便于维护
function codeCard(title, text) {
    // 返回计算结果或提前结束当前流程
    return `<div><h3 class="text-lg font-bold text-gray-800">${title}</h3><pre class="code-block mt-3 overflow-x-auto whitespace-pre-wrap break-words rounded-xl bg-gray-800 p-4 text-sm leading-6 text-gray-100">${escapeHtml(text || "")}</pre></div>`;
}

// 页面辅助函数：拆分复杂交互，便于维护
function renderTestCases(testCases) {
    const samples = testCases.filter((testCase) => testCase.isSample);
    // 返回计算结果或提前结束当前流程
    return `
        <section class="mt-6 border-t border-gray-200 pt-5">
            <div class="flex items-center justify-between gap-3">
                <h3 class="text-lg font-bold text-gray-800">公开测试用例</h3>
                <span class="text-sm text-gray-400">${samples.length} 条样例 / 共 ${testCases.length} 条</span>
            </div>
            <div class="mt-3 space-y-3">
                ${samples.length ? samples.map((testCase, index) => `
                    <details class="rounded-xl border border-gray-200 px-4 py-3">
                        <summary class="cursor-pointer text-sm font-medium text-gray-700">样例用例 ${index + 1} · ${testCase.score || 0} 分</summary>
                        <div class="mt-3 grid gap-3 md:grid-cols-2">
                            ${codeCard("输入", testCase.input)}
                            ${codeCard("输出", testCase.output)}
                        </div>
                    </details>
                `).join("") : emptyHtml("暂无公开样例用例。")}
            </div>
        </section>
    `;
}

// 页面辅助函数：拆分复杂交互，便于维护
function openEditor(problem = null) {
    // 条件分支：根据当前页面状态做不同处理
    if (!state.token) {
        setNotice("请先保存管理员 JWT，再维护题目。", "error");
        els.adminToken.focus();
        return;
    }
    els.problemForm.reset();
    els.editorProblemId.value = problem?.id || "";
    els.editorTitle.textContent = problem ? `编辑题目 #${problem.id}` : "新建题目";
    els.titleInput.value = problem?.title || "";
    els.difficultyInput.value = problem?.difficulty || "easy";
    els.tagsInput.value = (problem?.tags || []).join(", ");
    els.timeLimitInput.value = problem?.timeLimit || 1000;
    els.memoryLimitInput.value = problem?.memoryLimit || 128;
    els.descriptionInput.value = problem?.description || "";
    els.inputDescriptionInput.value = problem?.inputDescription || "";
    els.outputDescriptionInput.value = problem?.outputDescription || "";
    els.sampleInputInput.value = problem?.sampleInput || "";
    els.sampleOutputInput.value = problem?.sampleOutput || "";
    els.publicInput.checked = problem?.isPublic !== false;
    els.testCaseEditorList.innerHTML = "";
    const testCases = problem?.testCases?.length ? problem.testCases : [{ input: "", output: "", isSample: true, score: 100 }];
    // 遍历集合，为每个元素绑定逻辑或生成视图
    testCases.forEach(addTestCaseEditor);
    els.editorModal.classList.remove("hidden");
    document.body.classList.add("overflow-hidden");
    els.titleInput.focus();
}

// 页面辅助函数：拆分复杂交互，便于维护
function closeEditor() {
    els.editorModal.classList.add("hidden");
    document.body.classList.remove("overflow-hidden");
}

// 页面辅助函数：拆分复杂交互，便于维护
function addTestCaseEditor(testCase = {}) {
    const wrapper = document.createElement("div");
    wrapper.className = "test-case-editor rounded-2xl border border-gray-200 bg-gray-50 p-4";
    wrapper.innerHTML = `
        <div class="flex items-center justify-between gap-3">
            <h4 class="test-case-label font-bold text-gray-700"></h4>
            <button type="button" class="remove-test-case text-sm font-medium text-red-500 hover:underline">移除</button>
        </div>
        <div class="mt-3 grid gap-3 md:grid-cols-2">
            <label>
                <span class="block text-sm font-medium text-gray-700">输入</span>
                <textarea required rows="4" class="test-case-input code-input mt-2 w-full resize-y rounded-xl border border-gray-300 bg-white px-4 py-3 text-sm outline-none focus:ring-2 focus:ring-indigo-400">${escapeHtml(testCase.input || "")}</textarea>
            </label>
            <label>
                <span class="block text-sm font-medium text-gray-700">输出</span>
                <textarea required rows="4" class="test-case-output code-input mt-2 w-full resize-y rounded-xl border border-gray-300 bg-white px-4 py-3 text-sm outline-none focus:ring-2 focus:ring-indigo-400">${escapeHtml(testCase.output || "")}</textarea>
            </label>
            <label>
                <span class="block text-sm font-medium text-gray-700">分值</span>
                <input type="number" min="0" required value="${Number(testCase.score) || 0}" class="test-case-score mt-2 w-full rounded-xl border border-gray-300 bg-white px-4 py-3 outline-none focus:ring-2 focus:ring-indigo-400">
            </label>
            <label class="flex items-center gap-3 self-end py-3 text-sm font-medium text-gray-700">
                <input type="checkbox" class="test-case-sample h-4 w-4 rounded border-gray-300 text-indigo-600 focus:ring-indigo-400" ${testCase.isSample ? "checked" : ""}>
                作为公开样例
            </label>
        </div>
    `;
    wrapper.querySelector(".remove-test-case").addEventListener("click", () => {
        // 条件分支：根据当前页面状态做不同处理
        if (els.testCaseEditorList.children.length === 1) {
            setNotice("至少需要保留一个测试用例。", "error");
            return;
        }
        wrapper.remove();
        updateTestCaseLabels();
    });
    els.testCaseEditorList.append(wrapper);
    updateTestCaseLabels();
}

// 页面辅助函数：拆分复杂交互，便于维护
function updateTestCaseLabels() {
    // 遍历集合，为每个元素绑定逻辑或生成视图
    els.testCaseEditorList.querySelectorAll(".test-case-label").forEach((label, index) => {
        label.textContent = `测试用例 ${index + 1}`;
    });
}

// 页面辅助函数：拆分复杂交互，便于维护
function collectPayload() {
    const tags = els.tagsInput.value.split(",").map((tag) => tag.trim()).filter(Boolean);
    const testCases = [...els.testCaseEditorList.querySelectorAll(".test-case-editor")].map((wrapper) => ({
        input: wrapper.querySelector(".test-case-input").value.trim(),
        output: wrapper.querySelector(".test-case-output").value.trim(),
        score: Number(wrapper.querySelector(".test-case-score").value),
        isSample: wrapper.querySelector(".test-case-sample").checked,
    }));
    // 返回计算结果或提前结束当前流程
    return {
        title: els.titleInput.value.trim(),
        difficulty: els.difficultyInput.value,
        tags: [...new Set(tags)],
        timeLimit: Number(els.timeLimitInput.value),
        memoryLimit: Number(els.memoryLimitInput.value),
        description: els.descriptionInput.value.trim(),
        inputDescription: els.inputDescriptionInput.value.trim(),
        outputDescription: els.outputDescriptionInput.value.trim(),
        sampleInput: els.sampleInputInput.value.trim(),
        sampleOutput: els.sampleOutputInput.value.trim(),
        isPublic: els.publicInput.checked,
        testCases,
    };
}

// 异步流程：通常负责接口请求与界面回填
async function submitProblem(event) {
    event.preventDefault();
    const problemId = Number(els.editorProblemId.value) || null;
    try {
        const mutation = await requestJson(problemId ? `${API_BASE}/${problemId}` : API_BASE, {
            method: problemId ? "PUT" : "POST",
            headers: authHeaders(),
            body: JSON.stringify(collectPayload()),
        });
        closeEditor();
        setNotice(problemId ? "题目更新成功。" : "题目创建成功。", "success");
        await loadProblems({ keepSelection: true });
        await selectProblem(mutation.id);
    } catch (error) {
        setNotice(error.message, "error");
    }
}

// 异步流程：通常负责接口请求与界面回填
async function deleteProblem(problem) {
    // 条件分支：根据当前页面状态做不同处理
    if (!window.confirm(`确定删除题目 #${problem.id}「${problem.title}」吗？此操作无法撤销。`)) return;
    try {
        await requestJson(`${API_BASE}/${problem.id}`, {
            method: "DELETE",
            headers: authHeaders(),
        });
        state.selectedProblemId = null;
        state.selectedProblem = null;
        setNotice("题目删除成功。", "success");
        await loadProblems();
    } catch (error) {
        setNotice(error.message, "error");
    }
}

// 页面辅助函数：拆分复杂交互，便于维护
function formatRate(rate) {
    // 返回计算结果或提前结束当前流程
    return `${((Number(rate) || 0) * 100).toFixed(1)}%`;
}

// 页面辅助函数：拆分复杂交互，便于维护
function formatDateTime(value) {
    // 条件分支：根据当前页面状态做不同处理
    if (!value) {
        // 返回计算结果或提前结束当前流程
        return "-";
    }
    const date = new Date(value);
    // 条件分支：根据当前页面状态做不同处理
    if (Number.isNaN(date.getTime())) {
        // 返回计算结果或提前结束当前流程
        return "-";
    }
    // 返回计算结果或提前结束当前流程
    return date.toLocaleString();
}

// 页面辅助函数：拆分复杂交互，便于维护
function escapeHtml(value) {
    // 返回计算结果或提前结束当前流程
    return String(value ?? "")
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}

// 页面辅助函数：拆分复杂交互，便于维护
function bindEvents() {
    els.refreshButton.addEventListener("click", () => loadProblems({ keepSelection: true }).catch(handleError));
    els.filterForm.addEventListener("submit", (event) => {
        event.preventDefault();
        state.keyword = els.keywordFilter.value.trim();
        state.difficulty = els.difficultyFilter.value;
        state.tag = els.tagFilter.value.trim();
        state.page = 1;
        loadProblems().catch(handleError);
    });
    els.resetFilterButton.addEventListener("click", () => {
        els.filterForm.reset();
        state.keyword = "";
        state.difficulty = "";
        state.tag = "";
        state.page = 1;
        loadProblems().catch(handleError);
    });
    els.pageSize.addEventListener("change", () => {
        state.size = Number(els.pageSize.value);
        state.page = 1;
        loadProblems().catch(handleError);
    });
    els.previousPageButton.addEventListener("click", () => {
        // 条件分支：根据当前页面状态做不同处理
        if (state.page > 1) {
            state.page -= 1;
            loadProblems().catch(handleError);
        }
    });
    els.nextPageButton.addEventListener("click", () => {
        // 条件分支：根据当前页面状态做不同处理
        if (state.page * state.size < state.total) {
            state.page += 1;
            loadProblems().catch(handleError);
        }
    });
    els.saveTokenButton.addEventListener("click", () => {
        state.token = els.adminToken.value.trim().replace(/^Bearer\s+/i, "");
        // 条件分支：根据当前页面状态做不同处理
        if (state.token) {
            window.localStorage.setItem(TOKEN_STORAGE_KEY, state.token);
            setNotice("管理员 JWT 已保存在当前浏览器。", "success");
        } else {
            window.localStorage.removeItem(TOKEN_STORAGE_KEY);
            setNotice("JWT 已清除。", "info");
        }
        updateAdminStatus();
    });
    els.clearTokenButton.addEventListener("click", () => {
        state.token = "";
        els.adminToken.value = "";
        window.localStorage.removeItem(TOKEN_STORAGE_KEY);
        updateAdminStatus();
        setNotice("JWT 已清除。", "info");
    });
    els.createProblemButton.addEventListener("click", () => openEditor());
    els.closeEditorButton.addEventListener("click", closeEditor);
    els.cancelEditorButton.addEventListener("click", closeEditor);
    els.editorModal.addEventListener("click", (event) => {
        // 条件分支：根据当前页面状态做不同处理
        if (event.target === els.editorModal) closeEditor();
    });
    document.addEventListener("keydown", (event) => {
        // 条件分支：根据当前页面状态做不同处理
        if (event.key === "Escape" && !els.editorModal.classList.contains("hidden")) closeEditor();
    });
    els.addTestCaseButton.addEventListener("click", () => addTestCaseEditor({ input: "", output: "", isSample: false, score: 0 }));
    els.problemForm.addEventListener("submit", submitProblem);
}

// 页面辅助函数：拆分复杂交互，便于维护
function handleError(error) {
    setNotice(error.message, "error");
}

initFromQuery();
bindEvents();
loadProblems().catch(handleError);
