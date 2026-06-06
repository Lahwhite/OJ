const state = {
    // 当前已经加载到页面上的主题列表，翻页时追加，筛选时清空。
    topics: [],
    // 右侧详情面板当前选中的主题 id，null 表示还没有选择主题。
    selectedTopicId: null,
    // 列表分页偏移量始终等于 state.topics.length，避免重复加载。
    offset: 0,
    // 每次加载的主题数量，和后端默认值保持一致。
    limit: 20,
    // 从题目详情跳转过来时会预填 problem_id，只看当前题目的讨论。
    filterProblemId: null,
    // 用户模块通过 query string 传入当前登录用户名，删除和发布都依赖它。
    currentUsername: "",
    // AI 摘要按主题缓存，切换主题时不用重复调用 Gemini。
    summaryCache: new Map(),
};

// 所有 DOM 引用集中管理，后续改 HTML id 时可以在这里一次性排查。
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

// 通知条的基础样式固定，setNotice 只替换状态色和内容。
const NOTICE_BASE_CLASS = "mb-5 max-w-xl break-words rounded-xl border px-4 py-3 text-sm leading-6";
// 用户模块默认运行端口，头像和登录跳转无法从 query 推断时使用它。
const DEFAULT_USERS_ORIGIN = "http://127.0.0.1:8081";

function getQueryValue(name) {
    // URLSearchParams 会自动处理编码，避免手写 query 解析。
    return new URLSearchParams(window.location.search).get(name);
}

function initFromQuery() {
    // username 由用户模块跳转时携带，本页不单独维护登录态。
    state.currentUsername = getQueryValue("username") || "";
    const returnUrl = getQueryValue("return_url");
    if (returnUrl && isAllowedReturnUrl(returnUrl)) {
        // return_url 允许用户回到原来的用户主页或题目页。
        els.homeLink.href = returnUrl;
    }

    const problemId = Number(getQueryValue("problem_id"));
    if (Number.isInteger(problemId) && problemId > 0) {
        // 带题目编号进入讨论区时，筛选框和发帖框都同步预填。
        state.filterProblemId = problemId;
        els.problemFilter.value = String(problemId);
        els.topicProblemId.value = String(problemId);
    }
}

function isAllowedReturnUrl(value) {
    try {
        const url = new URL(value, window.location.href);
        // 只接受 http/https 或当前页面协议，避免 javascript: 之类的跳转。
        return url.protocol === "http:" || url.protocol === "https:" || url.protocol === window.location.protocol;
    } catch {
        return false;
    }
}

function displayName(item) {
    // 展示优先级：昵称 > 用户名 > 用户 id，保证任何数据都能显示作者。
    return item.nickname || item.username || `用户 #${item.user_id || "-"}`;
}

function usersAssetOrigin() {
    const explicitOrigin = getQueryValue("users_origin");
    if (explicitOrigin) {
        try {
            const url = new URL(explicitOrigin, window.location.href);
            if (url.protocol === "http:" || url.protocol === "https:") {
                // users_origin 只取 origin，避免路径污染头像拼接。
                return url.origin;
            }
        } catch {
            // 解析失败时继续使用 homeLink 推断，不打断页面初始化。
        }
    }

    try {
        // homeLink 通常来自用户模块，因此可作为头像资源的同源基准。
        return new URL(els.homeLink.href, window.location.href).origin;
    } catch {
        // 最后兜底到本地用户模块默认端口。
        return DEFAULT_USERS_ORIGIN;
    }
}

function defaultAvatarUrl() {
    // 默认头像通过后端 /images 代理读取用户模块静态资源。
    return "/images/default-avatar.png";
}

function avatarUrl(item) {
    // 后端可能返回绝对 URL、站内路径或用户模块相对路径，这里统一成浏览器可访问地址。
    const raw = String(item?.avatar || "").trim();
    if (!raw) {
        return defaultAvatarUrl();
    }
    if (/^(https?:|data:image\/)/i.test(raw)) {
        // 绝对地址和 data:image 已经可直接显示。
        return raw;
    }
    if (raw.startsWith("/uploads/avatars/") || raw.startsWith("/images/")) {
        // 已经是讨论区后端代理路径时直接使用。
        return raw;
    }
    if (raw.startsWith("uploads/avatars/") || raw.startsWith("images/")) {
        // 相对代理路径补一个前导斜杠。
        return `/${raw}`;
    }
    if (raw.startsWith("/")) {
        // 其他站内绝对路径交给当前服务处理。
        return raw;
    }
    // 用户模块返回的普通相对路径，需要拼到用户模块 origin 上。
    return `${usersAssetOrigin()}/${raw.replace(/^\.?\//, "")}`;
}

function avatarInitial(item) {
    // 图片加载失败时用用户名首字母兜底，避免头像区域塌陷。
    const name = displayName(item).trim();
    return name ? name.slice(0, 1).toUpperCase() : "U";
}

function avatarHtml(item, sizeClass = "") {
    // 图片和 fallback 同时渲染，失败事件会把图片移除并显示文字头像。
    const name = displayName(item);
    return `
        <span class="avatar ${sizeClass}">
            <img class="avatar-img" src="${escapeHtml(avatarUrl(item))}" alt="${escapeHtml(`${name} 的头像`)}" loading="lazy" referrerpolicy="no-referrer">
            <span class="avatar-fallback" aria-hidden="true">${escapeHtml(avatarInitial(item))}</span>
        </span>
    `;
}

function userInlineHtml(item, sizeClass = "") {
    // 作者展示在主题卡片、详情和评论里复用，保证头像和名称结构一致。
    return `
        ${avatarHtml(item, sizeClass)}
        <span class="truncate">${escapeHtml(displayName(item))}</span>
    `;
}

function bindAvatarFallbacks(root) {
    // 每次 innerHTML 重绘后都要重新绑定图片错误事件。
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
    // 空消息表示隐藏通知条，避免旧错误停留在页面上。
    if (!message) {
        els.notice.className = `${NOTICE_BASE_CLASS} hidden`;
        els.notice.textContent = "";
        return;
    }

    const themes = {
        // 三种状态色和 Tailwind 类写在一起，方便统一调整视觉反馈。
        info: "border-blue-200 bg-blue-50 text-blue-700",
        success: "border-green-200 bg-green-50 text-green-700",
        error: "border-red-200 bg-red-50 text-red-700",
    };
    els.notice.className = `${NOTICE_BASE_CLASS} ${themes[type] || themes.info}`;
    els.notice.textContent = message;
}

function formatDate(value) {
    // 后端已经格式化为本地时间字符串，这里只兼容 ISO 中的 T 分隔符。
    if (!value) {
        return "";
    }
    return value.replace("T", " ").slice(0, 19);
}

async function requestJson(url, options = {}) {
    // 统一 fetch 包装，所有接口失败都转成 Error，调用处只需 catch。
    const response = await fetch(url, {
        headers: {
            "Content-Type": "application/json",
            ...(options.headers || {}),
        },
        ...options,
    });

    const text = await response.text();
    // DELETE 等接口也可能返回空响应，先判断再 JSON.parse。
    const data = text ? JSON.parse(text) : null;
    if (!response.ok) {
        // 后端错误格式为 {message,error_code}，优先展示可读 message。
        const message = data?.message || data?.error_code || `请求失败：${response.status}`;
        throw new Error(message);
    }
    return data;
}

function topicUrl(reset = false) {
    // 列表 URL 只在这里拼，保证刷新和加载更多使用相同参数。
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
        // 重置加载用于刷新、筛选和增删后重新同步列表。
        state.offset = 0;
        state.topics = [];
    }

    setNotice("");
    els.topicList.innerHTML = state.topics.length ? els.topicList.innerHTML : loadingHtml("正在加载主题...");
    // 后端返回的是本页新增的一段数据，reset 时替换，否则追加。
    const topics = await requestJson(topicUrl(reset));
    state.topics = reset ? topics : [...state.topics, ...topics];
    state.offset = state.topics.length;
    renderTopics();

    els.loadMoreButton.classList.toggle("hidden", topics.length < state.limit);
    // 筛选或删除后，原先选中的主题可能已经不在当前列表里。
    const selectedStillVisible = state.topics.some((topic) => topic.id === state.selectedTopicId);
    if (state.selectedTopicId && !selectedStillVisible) {
        state.selectedTopicId = null;
    }

    if (!state.selectedTopicId && state.topics.length > 0) {
        // 首次加载自动选中第一条，让详情区域不空着。
        await selectTopic(state.topics[0].id);
    } else if (state.selectedTopicId) {
        markActiveTopic();
    } else {
        renderSummaryState(null);
    }
}

function loadingHtml(text) {
    // 加载占位保持和空状态相同尺寸，减少列表抖动。
    return `<div class="rounded-xl border border-gray-200 bg-gray-50 px-4 py-8 text-center text-sm text-gray-400">${text}</div>`;
}

function emptyHtml(text) {
    // 空状态统一样式，列表和评论区都复用。
    return `<div class="rounded-xl border border-dashed border-gray-300 px-4 py-8 text-center text-sm text-gray-400">${text}</div>`;
}

function renderTopics() {
    // 主题数量显示当前已加载数量，不代表数据库总数。
    els.topicCount.textContent = `${state.topics.length} 条`;
    if (state.topics.length === 0) {
        els.topicList.innerHTML = emptyHtml("暂无主题，发布一个新讨论吧。");
        return;
    }

    // 主题卡片使用 button，键盘和点击都能触发选择。
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
    // innerHTML 会清空旧监听器，所以每次渲染后重新绑定点击事件。
    els.topicList.querySelectorAll("[data-topic-id]").forEach((button) => {
        button.addEventListener("click", () => selectTopic(Number(button.dataset.topicId)));
    });
}

function markActiveTopic() {
    // 只切换 class，不重新渲染列表，避免滚动位置被重置。
    els.topicList.querySelectorAll("[data-topic-id]").forEach((button) => {
        button.classList.toggle("is-active", Number(button.dataset.topicId) === state.selectedTopicId);
    });
}

async function selectTopic(topicId) {
    // 先更新选中态，再并发加载详情和评论，用户能立即看到页面响应。
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

    // 主题正文和评论互不依赖，可以并发减少等待时间。
    const [topic, comments] = await Promise.all([
        requestJson(`/api/discussions/topics/${topicId}`),
        requestJson(`/api/discussions/topics/${topicId}/comments`),
    ]);

    renderDetail(topic);
    renderComments(comments);
}

function renderDetail(topic) {
    // 详情面板只使用 textContent 写正文，避免用户输入中的 HTML 被执行。
    els.detailProblem.textContent = `题目 ${topic.problem_id}`;
    els.detailAuthor.innerHTML = userInlineHtml(topic, "avatar-sm");
    els.detailTime.textContent = formatDate(topic.created_at);
    els.detailTitle.textContent = topic.title;
    els.detailContent.textContent = topic.content;
    // 当前用户只能删除自己发布的主题，按钮显示逻辑和后端权限校验双保险。
    const canDelete = state.currentUsername && topic.username === state.currentUsername;
    els.deleteTopicButton.classList.toggle("hidden", !canDelete);
    els.deleteTopicButton.dataset.topicAuthor = displayName(topic);
    renderSummaryState(topic.id);
    bindAvatarFallbacks(els.detailAuthor);
}

function renderSummaryState(topicId) {
    // 没有选中主题时禁用总结按钮，并清空上一个主题的摘要显示。
    if (!topicId) {
        els.summaryButton.disabled = true;
        els.summaryButton.textContent = "生成总结";
        els.summaryStatus.textContent = "选择主题后可生成当前讨论摘要。";
        els.summaryPanel.classList.add("hidden");
        els.summaryPanel.textContent = "";
        return;
    }

    const cached = state.summaryCache.get(topicId);
    // 有缓存时显示重新生成，表示用户可以主动刷新 Gemini 摘要。
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

    // 调用外部模型时把按钮锁住，防止重复点击产生多次请求。
    els.summaryButton.disabled = true;
    els.summaryButton.textContent = "生成中...";
    els.summaryStatus.textContent = "正在调用 Gemini 总结当前讨论...";
    els.summaryPanel.classList.remove("hidden");
    els.summaryPanel.textContent = "请稍候。";

    try {
        // 摘要接口从后端读取主题和评论，前端不传讨论正文，避免数据被篡改。
        const result = await requestJson(`/api/discussions/topics/${state.selectedTopicId}/summary`, {
            method: "POST",
            body: JSON.stringify({}),
        });
        state.summaryCache.set(state.selectedTopicId, result);
        renderSummaryState(state.selectedTopicId);
        setNotice("AI 总结已生成。", "success");
    } catch (error) {
        // 失败信息直接显示在摘要面板，便于知道是未配置还是网络错误。
        els.summaryStatus.textContent = "总结失败，请稍后重试。";
        els.summaryPanel.textContent = error.message;
        setNotice(error.message, "error");
    } finally {
        // 如果失败且没有缓存，按钮文案恢复成首次生成。
        els.summaryButton.disabled = false;
        if (!state.summaryCache.has(state.selectedTopicId)) {
            els.summaryButton.textContent = "生成总结";
        }
    }
}

function renderComments(comments) {
    // 评论数量来自当前接口返回长度，删除回复树后会自然减少。
    els.commentCount.textContent = `${comments.length} 条`;
    if (comments.length === 0) {
        els.commentList.innerHTML = emptyHtml("还没有评论。");
        return;
    }

    const byId = new Map(comments.map((comment) => [comment.id, comment]));
    els.commentList.innerHTML = comments.map((comment) => {
        // 后端按 id 升序返回，因此父评论通常已经在 byId 中。
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
        // 删除按钮只给本人显示；即使前端被篡改，后端仍会做作者校验。
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
    // 回复和删除按钮都是动态生成的，需要在渲染后绑定。
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
    // 删除父评论会连带删除子回复，确认文案需要明确告知用户。
    const confirmed = window.confirm(`确定删除 ${author} 的这条评论吗？如果它有回复，回复也会一起删除。`);
    if (!confirmed) {
        return;
    }

    try {
        // 后端会返回删除数量，包含子回复。
        const result = await requestJson(`/api/discussions/topics/${state.selectedTopicId}/comments/${commentId}`, {
            method: "DELETE",
            body: JSON.stringify({ username }),
        });
        // 内容变更后旧摘要失效，下次打开或点击时重新生成。
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
    // 主题删除会清空整棵评论树，前端先做显式确认。
    const confirmed = window.confirm(`确定删除 ${author} 的这个主题吗？主题下的评论和回复也会一起删除。`);
    if (!confirmed) {
        return;
    }

    try {
        // 成功删除后清空详情面板，再重新加载列表。
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
    // 回复状态通过 hidden input 带入提交 payload，提示区显示当前回复对象。
    els.parentCommentId.value = String(commentId);
    els.replyHint.textContent = `正在回复 ${author}`;
    els.replyHint.classList.remove("hidden");
    els.cancelReplyButton.classList.remove("hidden");
    els.commentContent.focus();
}

function clearReply() {
    // 切换主题、提交成功或取消时都要清除回复上下文。
    els.parentCommentId.value = "";
    els.replyHint.textContent = "";
    els.replyHint.classList.add("hidden");
    els.cancelReplyButton.classList.add("hidden");
}

function requireUsername() {
    if (state.currentUsername) {
        return state.currentUsername;
    }

    // 没有 username 说明不是从已登录用户页进入，跳回登录页重新建立身份。
    const loginUrl = new URL("/login", usersAssetOrigin());
    window.location.assign(loginUrl.toString());
    throw new Error("User not logged in, redirecting to login.");
}

async function submitTopic(event) {
    event.preventDefault();
    try {
        // 发布主题只传 username，服务端负责转换成 user_id 并校验用户存在。
        const payload = {
            problem_id: Number(els.topicProblemId.value),
            username: requireUsername(),
            title: els.topicTitle.value.trim(),
            content: els.topicContent.value.trim(),
        };

        if (!payload.problem_id || !payload.title || !payload.content) {
            // 浏览器 required 可以兜底，这里保留 JS 层提示用于脚本提交。
            throw new Error("请填写完整的主题信息。");
        }

        // 创建成功后刷新列表并选中新主题，保证评论区立即可用。
        const created = await requestJson("/api/discussions/topics", {
            method: "POST",
            body: JSON.stringify(payload),
        });
        els.topicForm.reset();
        if (state.filterProblemId) {
            // 当前筛选某题时，发布框继续保留该题编号。
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
        // parent_comment_id 为 null 表示普通评论，为数字表示回复。
        const payload = {
            username: requireUsername(),
            content: els.commentContent.value.trim(),
            parent_comment_id: els.parentCommentId.value ? Number(els.parentCommentId.value) : null,
        };

        if (!payload.content) {
            // 去掉首尾空白后再判断，避免提交只有空格的评论。
            throw new Error("评论内容不能为空。");
        }

        // 评论会影响评论数和主题更新时间，所以提交后刷新列表与详情。
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
    // 所有拼进 innerHTML 的用户内容都必须先转义。
    return String(value ?? "")
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}

function bindEvents() {
    // 顶部刷新只重拉主题列表，保留当前筛选条件。
    els.refreshButton.addEventListener("click", () => loadTopics({ reset: true }).catch((error) => setNotice(error.message, "error")));
    // 加载更多使用当前 offset 继续分页。
    els.loadMoreButton.addEventListener("click", () => loadTopics().catch((error) => setNotice(error.message, "error")));
    els.summaryButton.addEventListener("click", () => summarizeTopic());
    els.deleteTopicButton.addEventListener("click", () => deleteTopic());
    els.topicForm.addEventListener("submit", submitTopic);
    els.commentForm.addEventListener("submit", submitComment);
    els.cancelReplyButton.addEventListener("click", clearReply);
    els.filterForm.addEventListener("submit", (event) => {
        event.preventDefault();
        // 空值或非法数字表示取消题目过滤。
        const problemId = Number(els.problemFilter.value);
        state.filterProblemId = Number.isInteger(problemId) && problemId > 0 ? problemId : null;
        if (state.filterProblemId) {
            els.topicProblemId.value = String(state.filterProblemId);
        }
        state.selectedTopicId = null;
        // 切换过滤条件后关闭详情，避免显示不属于当前列表的主题。
        els.topicDetail.classList.add("hidden");
        els.emptyDetail.classList.remove("hidden");
        renderSummaryState(null);
        loadTopics({ reset: true }).catch((error) => setNotice(error.message, "error"));
    });
}

// 初始化顺序：先读取 query，再绑定事件，最后加载首屏数据。
initFromQuery();
bindEvents();
renderSummaryState(null);
loadTopics({ reset: true }).catch((error) => setNotice(error.message, "error"));
