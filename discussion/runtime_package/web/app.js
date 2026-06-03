const state = {
    topics: [],
    selectedTopicId: null,
    offset: 0,
    limit: 20,
    filterProblemId: null,
    currentUsername: "",
    summaryCache: new Map(),
};

const els = {
    notice: document.querySelector("#notice"),
    refreshButton: document.querySelector("#refreshButton"),
    filterForm: document.querySelector("#filterForm"),
    homeLink: document.querySelector("#homeLink"),
    problemFilter: document.querySelector("#problemFilter"),
    topicForm: document.querySelector("#topicForm"),
    topicProblemId: document.querySelector("#topicProblemId"),
    topicTitle: document.querySelector("#topicTitle"),
    topicContent: document.querySelector("#topicContent"),
    topicList: document.querySelector("#topicList"),
    topicCount: document.querySelector("#topicCount"),
    loadMoreButton: document.querySelector("#loadMoreButton"),
    emptyDetail: document.querySelector("#emptyDetail"),
    topicDetail: document.querySelector("#topicDetail"),
    detailProblem: document.querySelector("#detailProblem"),
    detailAuthor: document.querySelector("#detailAuthor"),
    detailTime: document.querySelector("#detailTime"),
    detailTitle: document.querySelector("#detailTitle"),
    detailContent: document.querySelector("#detailContent"),
    deleteTopicButton: document.querySelector("#deleteTopicButton"),
    summaryButton: document.querySelector("#summaryButton"),
    summaryStatus: document.querySelector("#summaryStatus"),
    summaryPanel: document.querySelector("#summaryPanel"),
    commentCount: document.querySelector("#commentCount"),
    commentList: document.querySelector("#commentList"),
    commentForm: document.querySelector("#commentForm"),
    commentContent: document.querySelector("#commentContent"),
    parentCommentId: document.querySelector("#parentCommentId"),
    replyHint: document.querySelector("#replyHint"),
    cancelReplyButton: document.querySelector("#cancelReplyButton"),
};

const NOTICE_BASE_CLASS = "mb-5 max-w-xl break-words rounded-xl border px-4 py-3 text-sm leading-6";
const DEFAULT_USERS_ORIGIN = "http://127.0.0.1:8081";

function getQueryValue(name) {
    return new URLSearchParams(window.location.search).get(name);
}

function initFromQuery() {
    state.currentUsername = getQueryValue("username") || "";
    const returnUrl = getQueryValue("return_url");
    if (returnUrl && isAllowedReturnUrl(returnUrl)) {
        els.homeLink.href = returnUrl;
    }

    const problemId = Number(getQueryValue("problem_id"));
    if (Number.isInteger(problemId) && problemId > 0) {
        state.filterProblemId = problemId;
        els.problemFilter.value = String(problemId);
        els.topicProblemId.value = String(problemId);
    }
}

function isAllowedReturnUrl(value) {
    try {
        const url = new URL(value, window.location.href);
        return url.protocol === "http:" || url.protocol === "https:" || url.protocol === window.location.protocol;
    } catch {
        return false;
    }
}

function displayName(item) {
    return item.nickname || item.username || `用户 #${item.user_id || "-"}`;
}

function usersAssetOrigin() {
    const explicitOrigin = getQueryValue("users_origin");
    if (explicitOrigin) {
        try {
            const url = new URL(explicitOrigin, window.location.href);
            if (url.protocol === "http:" || url.protocol === "https:") {
                return url.origin;
            }
        } catch {
            // Fall back to the configured home link below.
        }
    }

    try {
        return new URL(els.homeLink.href, window.location.href).origin;
    } catch {
        return DEFAULT_USERS_ORIGIN;
    }
}

function defaultAvatarUrl() {
    return "/images/default-avatar.png";
}

function avatarUrl(item) {
    const raw = String(item?.avatar || "").trim();
    if (!raw) {
        return defaultAvatarUrl();
    }
    if (/^(https?:|data:image\/)/i.test(raw)) {
        return raw;
    }
    if (raw.startsWith("/uploads/avatars/") || raw.startsWith("/images/")) {
        return raw;
    }
    if (raw.startsWith("uploads/avatars/") || raw.startsWith("images/")) {
        return `/${raw}`;
    }
    if (raw.startsWith("/")) {
        return raw;
    }
    return `${usersAssetOrigin()}/${raw.replace(/^\.?\//, "")}`;
}

function avatarInitial(item) {
    const name = displayName(item).trim();
    return name ? name.slice(0, 1).toUpperCase() : "U";
}

function avatarHtml(item, sizeClass = "") {
    const name = displayName(item);
    return `
        <span class="avatar ${sizeClass}">
            <img class="avatar-img" src="${escapeHtml(avatarUrl(item))}" alt="${escapeHtml(`${name} 的头像`)}" loading="lazy" referrerpolicy="no-referrer">
            <span class="avatar-fallback" aria-hidden="true">${escapeHtml(avatarInitial(item))}</span>
        </span>
    `;
}

function userInlineHtml(item, sizeClass = "") {
    return `
        ${avatarHtml(item, sizeClass)}
        <span class="truncate">${escapeHtml(displayName(item))}</span>
    `;
}

function bindAvatarFallbacks(root) {
    root.querySelectorAll(".avatar-img").forEach((img) => {
        img.addEventListener("error", () => {
            const avatar = img.closest(".avatar");
            if (avatar) {
                avatar.classList.add("is-fallback");
            }
            img.remove();
        }, { once: true });
    });
}

function setNotice(message, type = "info") {
    if (!message) {
        els.notice.className = `${NOTICE_BASE_CLASS} hidden`;
        els.notice.textContent = "";
        return;
    }

    const themes = {
        info: "border-blue-200 bg-blue-50 text-blue-700",
        success: "border-green-200 bg-green-50 text-green-700",
        error: "border-red-200 bg-red-50 text-red-700",
    };
    els.notice.className = `${NOTICE_BASE_CLASS} ${themes[type] || themes.info}`;
    els.notice.textContent = message;
}

function formatDate(value) {
    if (!value) {
        return "";
    }
    return value.replace("T", " ").slice(0, 19);
}

async function requestJson(url, options = {}) {
    const response = await fetch(url, {
        headers: {
            "Content-Type": "application/json",
            ...(options.headers || {}),
        },
        ...options,
    });

    const text = await response.text();
    const data = text ? JSON.parse(text) : null;
    if (!response.ok) {
        const message = data?.message || data?.error_code || `请求失败：${response.status}`;
        throw new Error(message);
    }
    return data;
}

function topicUrl(reset = false) {
    const params = new URLSearchParams();
    params.set("limit", String(state.limit));
    params.set("offset", String(reset ? 0 : state.offset));
    if (state.filterProblemId) {
        params.set("problem_id", String(state.filterProblemId));
    }
    return `/api/discussions/topics?${params.toString()}`;
}

async function loadTopics({ reset = false } = {}) {
    if (reset) {
        state.offset = 0;
        state.topics = [];
    }

    setNotice("");
    els.topicList.innerHTML = state.topics.length ? els.topicList.innerHTML : loadingHtml("正在加载主题...");
    const topics = await requestJson(topicUrl(reset));
    state.topics = reset ? topics : [...state.topics, ...topics];
    state.offset = state.topics.length;
    renderTopics();

    els.loadMoreButton.classList.toggle("hidden", topics.length < state.limit);
    const selectedStillVisible = state.topics.some((topic) => topic.id === state.selectedTopicId);
    if (state.selectedTopicId && !selectedStillVisible) {
        state.selectedTopicId = null;
    }

    if (!state.selectedTopicId && state.topics.length > 0) {
        await selectTopic(state.topics[0].id);
    } else if (state.selectedTopicId) {
        markActiveTopic();
    } else {
        renderSummaryState(null);
    }
}

function loadingHtml(text) {
    return `<div class="rounded-xl border border-gray-200 bg-gray-50 px-4 py-8 text-center text-sm text-gray-400">${text}</div>`;
}

function emptyHtml(text) {
    return `<div class="rounded-xl border border-dashed border-gray-300 px-4 py-8 text-center text-sm text-gray-400">${text}</div>`;
}

function renderTopics() {
    els.topicCount.textContent = `${state.topics.length} 条`;
    if (state.topics.length === 0) {
        els.topicList.innerHTML = emptyHtml("暂无主题，发布一个新讨论吧。");
        return;
    }

    els.topicList.innerHTML = state.topics.map((topic) => `
        <button type="button" class="topic-card w-full bg-white text-left ${topic.id === state.selectedTopicId ? "is-active" : ""}" data-topic-id="${topic.id}">
            <div class="flex items-center justify-between gap-3">
                <span class="rounded-lg bg-blue-50 px-3 py-1 text-xs font-medium text-blue-600">题目 ${topic.problem_id}</span>
                <span class="text-xs text-gray-400">${formatDate(topic.updated_at || topic.created_at)}</span>
            </div>
            <h3 class="mt-3 line-clamp-2 font-bold text-gray-800">${escapeHtml(topic.title)}</h3>
            <p class="mt-2 line-clamp-2 text-sm leading-6 text-gray-500">${escapeHtml(topic.content)}</p>
            <div class="mt-3 flex items-center justify-between gap-3 text-xs text-gray-400">
                <span class="inline-flex min-w-0 items-center gap-2 text-gray-500">
                    ${userInlineHtml(topic, "avatar-sm")}
                </span>
                <span class="shrink-0">${topic.comment_count || 0} 条评论</span>
            </div>
        </button>
    `).join("");

    bindAvatarFallbacks(els.topicList);
    els.topicList.querySelectorAll("[data-topic-id]").forEach((button) => {
        button.addEventListener("click", () => selectTopic(Number(button.dataset.topicId)));
    });
}

function markActiveTopic() {
    els.topicList.querySelectorAll("[data-topic-id]").forEach((button) => {
        button.classList.toggle("is-active", Number(button.dataset.topicId) === state.selectedTopicId);
    });
}

async function selectTopic(topicId) {
    state.selectedTopicId = topicId;
    markActiveTopic();
    clearReply();

    els.emptyDetail.classList.add("hidden");
    els.topicDetail.classList.remove("hidden");
    els.deleteTopicButton.classList.add("hidden");
    els.deleteTopicButton.dataset.topicAuthor = "";
    els.detailTitle.textContent = "正在加载...";
    els.detailContent.textContent = "";
    els.commentList.innerHTML = loadingHtml("正在加载评论...");
    renderSummaryState(topicId);

    const [topic, comments] = await Promise.all([
        requestJson(`/api/discussions/topics/${topicId}`),
        requestJson(`/api/discussions/topics/${topicId}/comments`),
    ]);

    renderDetail(topic);
    renderComments(comments);
}

function renderDetail(topic) {
    els.detailProblem.textContent = `题目 ${topic.problem_id}`;
    els.detailAuthor.innerHTML = userInlineHtml(topic, "avatar-sm");
    els.detailTime.textContent = formatDate(topic.created_at);
    els.detailTitle.textContent = topic.title;
    els.detailContent.textContent = topic.content;
    const canDelete = state.currentUsername && topic.username === state.currentUsername;
    els.deleteTopicButton.classList.toggle("hidden", !canDelete);
    els.deleteTopicButton.dataset.topicAuthor = displayName(topic);
    renderSummaryState(topic.id);
    bindAvatarFallbacks(els.detailAuthor);
}

function renderSummaryState(topicId) {
    if (!topicId) {
        els.summaryButton.disabled = true;
        els.summaryButton.textContent = "生成总结";
        els.summaryStatus.textContent = "选择主题后可生成当前讨论摘要。";
        els.summaryPanel.classList.add("hidden");
        els.summaryPanel.textContent = "";
        return;
    }

    const cached = state.summaryCache.get(topicId);
    els.summaryButton.disabled = false;
    els.summaryButton.textContent = cached ? "重新生成" : "生成总结";
    els.summaryStatus.textContent = cached ? `由 ${cached.model || "Gemini"} 生成` : "点击按钮后由 Gemini 生成当前主题和评论摘要。";
    els.summaryPanel.classList.toggle("hidden", !cached);
    els.summaryPanel.textContent = cached?.summary || "";
}

async function summarizeTopic() {
    if (!state.selectedTopicId) {
        return;
    }

    els.summaryButton.disabled = true;
    els.summaryButton.textContent = "生成中...";
    els.summaryStatus.textContent = "正在调用 Gemini 总结当前讨论...";
    els.summaryPanel.classList.remove("hidden");
    els.summaryPanel.textContent = "请稍候。";

    try {
        const result = await requestJson(`/api/discussions/topics/${state.selectedTopicId}/summary`, {
            method: "POST",
            body: JSON.stringify({}),
        });
        state.summaryCache.set(state.selectedTopicId, result);
        renderSummaryState(state.selectedTopicId);
        setNotice("AI 总结已生成。", "success");
    } catch (error) {
        els.summaryStatus.textContent = "总结失败，请稍后重试。";
        els.summaryPanel.textContent = error.message;
        setNotice(error.message, "error");
    } finally {
        els.summaryButton.disabled = false;
        if (!state.summaryCache.has(state.selectedTopicId)) {
            els.summaryButton.textContent = "生成总结";
        }
    }
}

function renderComments(comments) {
    els.commentCount.textContent = `${comments.length} 条`;
    if (comments.length === 0) {
        els.commentList.innerHTML = emptyHtml("还没有评论。");
        return;
    }

    const byId = new Map(comments.map((comment) => [comment.id, comment]));
    els.commentList.innerHTML = comments.map((comment) => {
        const parent = comment.parent_comment_id ? byId.get(comment.parent_comment_id) : null;
        const parentLine = parent ? `
            <div class="mb-2 rounded-lg bg-gray-50 px-3 py-2 text-sm text-gray-500">
                <span class="inline-flex min-w-0 items-center gap-2 align-middle">
                    <span>回复</span>
                    ${userInlineHtml(parent, "avatar-xs")}
                </span>
                <span>：${escapeHtml(parent.content).slice(0, 80)}</span>
            </div>
        ` : "";
        const canDelete = state.currentUsername && comment.username === state.currentUsername;
        const deleteButton = canDelete ? `<button type="button" class="delete-comment-button mt-2 text-sm font-medium text-red-400 hover:underline" data-comment-id="${comment.id}" data-comment-author="${escapeHtml(displayName(comment))}">删除</button>` : "";
        return `
            <div class="comment-card" data-comment-id="${comment.id}" data-comment-author="${escapeHtml(displayName(comment))}">
                <div class="flex flex-wrap items-center justify-between gap-2">
                    <div class="inline-flex min-w-0 items-center gap-2 font-medium text-gray-800">
                        ${userInlineHtml(comment)}
                    </div>
                    <div class="text-xs text-gray-400">${formatDate(comment.created_at)}</div>
                </div>
                ${parentLine}
                <p class="mt-2 whitespace-pre-wrap break-words leading-7 text-gray-700">${escapeHtml(comment.content)}</p>
                <div class="flex flex-wrap gap-3">
                    <button type="button" class="reply-button mt-2 text-sm font-medium text-blue-400 hover:underline" data-comment-id="${comment.id}" data-comment-author="${escapeHtml(displayName(comment))}">回复</button>
                    ${deleteButton}
                </div>
            </div>
        `;
    }).join("");

    bindAvatarFallbacks(els.commentList);
    els.commentList.querySelectorAll(".reply-button").forEach((button) => {
        button.addEventListener("click", () => setReply(Number(button.dataset.commentId), button.dataset.commentAuthor));
    });
    els.commentList.querySelectorAll(".delete-comment-button").forEach((button) => {
        button.addEventListener("click", () => deleteComment(Number(button.dataset.commentId), button.dataset.commentAuthor));
    });
}

async function deleteComment(commentId, author) {
    if (!state.selectedTopicId) {
        return;
    }

    const username = requireUsername();
    const confirmed = window.confirm(`确定删除 ${author} 的这条评论吗？如果它有回复，回复也会一起删除。`);
    if (!confirmed) {
        return;
    }

    try {
        const result = await requestJson(`/api/discussions/topics/${state.selectedTopicId}/comments/${commentId}`, {
            method: "DELETE",
            body: JSON.stringify({ username }),
        });
        state.summaryCache.delete(state.selectedTopicId);
        clearReply();
        setNotice(`已删除 ${result.deleted_count || 1} 条评论。`, "success");
        await loadTopics({ reset: true });
        await selectTopic(state.selectedTopicId);
    } catch (error) {
        setNotice(error.message, "error");
    }
}

async function deleteTopic() {
    if (!state.selectedTopicId) {
        return;
    }

    const topicId = state.selectedTopicId;
    const username = requireUsername();
    const author = els.deleteTopicButton.dataset.topicAuthor || "当前用户";
    const confirmed = window.confirm(`确定删除 ${author} 的这个主题吗？主题下的评论和回复也会一起删除。`);
    if (!confirmed) {
        return;
    }

    try {
        const result = await requestJson(`/api/discussions/topics/${topicId}`, {
            method: "DELETE",
            body: JSON.stringify({ username }),
        });
        state.summaryCache.delete(topicId);
        state.selectedTopicId = null;
        clearReply();
        els.topicDetail.classList.add("hidden");
        els.emptyDetail.classList.remove("hidden");
        renderSummaryState(null);
        await loadTopics({ reset: true });
        setNotice(`已删除 ${result.deleted_count || 1} 个主题。`, "success");
    } catch (error) {
        setNotice(error.message, "error");
    }
}

function setReply(commentId, author) {
    els.parentCommentId.value = String(commentId);
    els.replyHint.textContent = `正在回复 ${author}`;
    els.replyHint.classList.remove("hidden");
    els.cancelReplyButton.classList.remove("hidden");
    els.commentContent.focus();
}

function clearReply() {
    els.parentCommentId.value = "";
    els.replyHint.textContent = "";
    els.replyHint.classList.add("hidden");
    els.cancelReplyButton.classList.add("hidden");
}

function requireUsername() {
    if (state.currentUsername) {
        return state.currentUsername;
    }

    const typed = window.prompt("请输入当前登录用户名，用于创建讨论内容：");
    if (!typed || !typed.trim()) {
        throw new Error("缺少用户名，无法提交。");
    }
    state.currentUsername = typed.trim();
    const url = new URL(window.location.href);
    url.searchParams.set("username", state.currentUsername);
    window.history.replaceState({}, "", url);
    return state.currentUsername;
}

async function submitTopic(event) {
    event.preventDefault();
    try {
        const payload = {
            problem_id: Number(els.topicProblemId.value),
            username: requireUsername(),
            title: els.topicTitle.value.trim(),
            content: els.topicContent.value.trim(),
        };

        if (!payload.problem_id || !payload.title || !payload.content) {
            throw new Error("请填写完整的主题信息。");
        }

        const created = await requestJson("/api/discussions/topics", {
            method: "POST",
            body: JSON.stringify(payload),
        });
        els.topicForm.reset();
        if (state.filterProblemId) {
            els.topicProblemId.value = String(state.filterProblemId);
        }
        setNotice("主题发布成功。", "success");
        await loadTopics({ reset: true });
        await selectTopic(created.id);
    } catch (error) {
        setNotice(error.message, "error");
    }
}

async function submitComment(event) {
    event.preventDefault();
    if (!state.selectedTopicId) {
        return;
    }

    try {
        const payload = {
            username: requireUsername(),
            content: els.commentContent.value.trim(),
            parent_comment_id: els.parentCommentId.value ? Number(els.parentCommentId.value) : null,
        };

        if (!payload.content) {
            throw new Error("评论内容不能为空。");
        }

        await requestJson(`/api/discussions/topics/${state.selectedTopicId}/comments`, {
            method: "POST",
            body: JSON.stringify(payload),
        });
        state.summaryCache.delete(state.selectedTopicId);
        els.commentForm.reset();
        clearReply();
        setNotice("评论提交成功。", "success");
        await loadTopics({ reset: true });
        await selectTopic(state.selectedTopicId);
    } catch (error) {
        setNotice(error.message, "error");
    }
}

function escapeHtml(value) {
    return String(value ?? "")
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}

function bindEvents() {
    els.refreshButton.addEventListener("click", () => loadTopics({ reset: true }).catch((error) => setNotice(error.message, "error")));
    els.loadMoreButton.addEventListener("click", () => loadTopics().catch((error) => setNotice(error.message, "error")));
    els.summaryButton.addEventListener("click", () => summarizeTopic());
    els.deleteTopicButton.addEventListener("click", () => deleteTopic());
    els.topicForm.addEventListener("submit", submitTopic);
    els.commentForm.addEventListener("submit", submitComment);
    els.cancelReplyButton.addEventListener("click", clearReply);
    els.filterForm.addEventListener("submit", (event) => {
        event.preventDefault();
        const problemId = Number(els.problemFilter.value);
        state.filterProblemId = Number.isInteger(problemId) && problemId > 0 ? problemId : null;
        if (state.filterProblemId) {
            els.topicProblemId.value = String(state.filterProblemId);
        }
        state.selectedTopicId = null;
        els.topicDetail.classList.add("hidden");
        els.emptyDetail.classList.remove("hidden");
        renderSummaryState(null);
        loadTopics({ reset: true }).catch((error) => setNotice(error.message, "error"));
    });
}

initFromQuery();
bindEvents();
renderSummaryState(null);
loadTopics({ reset: true }).catch((error) => setNotice(error.message, "error"));
