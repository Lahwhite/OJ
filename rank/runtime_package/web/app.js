const state = {
    mode: "global",
    rows: [],
    selectedUserId: null,
    selectedStats: null,
    contestId: 1001,
    offset: 0,
    limit: 20,
    charts: {
        score: null,
        difficulty: null,
        combo: null,
    },
};

const els = {
    notice: document.querySelector("#notice"),
    homeLink: document.querySelector("#homeLink"),
    refreshButton: document.querySelector("#refreshButton"),
    tabGlobal: document.querySelector("#tabGlobal"),
    tabContest: document.querySelector("#tabContest"),
    contestForm: document.querySelector("#contestForm"),
    contestId: document.querySelector("#contestId"),
    podium: document.querySelector("#podium"),
    leaderboardBody: document.querySelector("#leaderboardBody"),
    rowCount: document.querySelector("#rowCount"),
    pageInfo: document.querySelector("#pageInfo"),
    prevPage: document.querySelector("#prevPage"),
    nextPage: document.querySelector("#nextPage"),
    statsHint: document.querySelector("#statsHint"),
    exportCsvButton: document.querySelector("#exportCsvButton"),
    summaryUsers: document.querySelector("#summaryUsers"),
    summaryMax: document.querySelector("#summaryMax"),
    summaryAvg: document.querySelector("#summaryAvg"),
    summaryMedian: document.querySelector("#summaryMedian"),
    summarySolved: document.querySelector("#summarySolved"),
    summaryPercentile: document.querySelector("#summaryPercentile"),
};

const LC_COLORS = {
    primary: "#ffa116",
    easy: "#00b8a3",
    medium: "#ffc01e",
    hard: "#ff375f",
    muted: "#8c8c8c",
    grid: "#eff0f2",
};

function getQueryValue(name) {
    return new URLSearchParams(window.location.search).get(name);
}

function initFromQuery() {
    const returnUrl = getQueryValue("return_url");
    if (returnUrl && isAllowedReturnUrl(returnUrl)) {
        els.homeLink.href = returnUrl;
    }

    const contestId = Number(getQueryValue("contest_id"));
    if (Number.isInteger(contestId) && contestId > 0) {
        state.contestId = contestId;
        els.contestId.value = String(contestId);
        setMode("contest");
    }

    const userId = Number(getQueryValue("user_id"));
    if (Number.isInteger(userId) && userId > 0) {
        state.selectedUserId = userId;
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

function setNotice(message, type = "info") {
    if (!message) {
        els.notice.className = "mb-4 hidden rounded-xl border px-4 py-3 text-sm";
        els.notice.textContent = "";
        return;
    }
    const themes = {
        info: "border-blue-200 bg-blue-50 text-blue-700",
        success: "border-green-200 bg-green-50 text-green-700",
        error: "border-red-200 bg-red-50 text-red-700",
    };
    els.notice.className = `mb-4 rounded-xl border px-4 py-3 text-sm ${themes[type] || themes.info}`;
    els.notice.textContent = message;
}

function apiUrl(path) {
    return `${window.location.origin}${path}`;
}

async function fetchJson(path) {
    const response = await fetch(apiUrl(path), { headers: { Accept: "application/json" } });
    if (!response.ok) {
        throw new Error(`请求失败 (${response.status})`);
    }
    return response.json();
}

function setMode(mode) {
    state.mode = mode;
    state.offset = 0;
    els.tabGlobal.classList.toggle("is-active", mode === "global");
    els.tabContest.classList.toggle("is-active", mode === "contest");
    els.contestForm.classList.toggle("hidden", mode !== "contest");
    loadLeaderboard();
}

function rankBadgeClass(rank) {
    if (rank === 1) return "is-gold";
    if (rank === 2) return "is-silver";
    if (rank === 3) return "is-bronze";
    return "";
}

function formatNumber(value) {
    return new Intl.NumberFormat("zh-CN").format(value ?? 0);
}

function formatPenalty(seconds) {
    if (!seconds) return "0";
    const h = Math.floor(seconds / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    if (h > 0) return `${h}h ${m}m`;
    return `${m}m`;
}

function renderPodium() {
    const top3 = state.rows.slice(0, 3);
    if (!top3.length) {
        els.podium.innerHTML = "";
        return;
    }

    const order = [1, 0, 2].filter((idx) => top3[idx]);
    els.podium.innerHTML = order
        .map((idx) => {
            const row = top3[idx];
            const rank = row.rank ?? idx + 1;
            return `
                <article class="lc-podium-card rank-${rank}">
                    <div class="text-xs font-semibold uppercase tracking-wide text-[var(--lc-muted)]">#${rank}</div>
                    <div class="mt-2 text-xl font-bold text-[var(--lc-text)]">${escapeHtml(row.username)}</div>
                    <div class="mt-4 flex items-end justify-between">
                        <div>
                            <div class="text-xs text-[var(--lc-muted)]">积分</div>
                            <div class="text-2xl font-extrabold" style="color:${LC_COLORS.primary}">${formatNumber(row.score)}</div>
                        </div>
                        <div class="text-right">
                            <div class="text-xs text-[var(--lc-muted)]">通过</div>
                            <div class="text-lg font-semibold">${formatNumber(row.solved_count)}</div>
                        </div>
                    </div>
                </article>
            `;
        })
        .join("");
}

function renderTable() {
    els.rowCount.textContent = `${state.rows.length} 人`;
    const page = Math.floor(state.offset / state.limit) + 1;
    els.pageInfo.textContent = `第 ${page} 页`;
    els.prevPage.disabled = state.offset <= 0;
    els.nextPage.disabled = state.rows.length < state.limit;

    els.leaderboardBody.innerHTML = state.rows
        .map((row) => {
            const rank = row.rank ?? "-";
            const selected = row.user_id === state.selectedUserId;
            const badge = rankBadgeClass(rank);
            return `
                <tr class="${selected ? "is-selected" : ""}" data-user-id="${row.user_id}">
                    <td class="px-5 py-3">
                        <span class="lc-rank-badge ${badge}">${rank}</span>
                    </td>
                    <td class="px-5 py-3 font-medium">${escapeHtml(row.username)}</td>
                    <td class="px-5 py-3 text-right font-semibold" style="color:${LC_COLORS.primary}">${formatNumber(row.score)}</td>
                    <td class="px-5 py-3 text-right">${formatNumber(row.solved_count)}</td>
                    <td class="px-5 py-3 text-right hidden sm:table-cell">${formatPenalty(row.penalty_seconds)}</td>
                    <td class="px-5 py-3 text-right hidden md:table-cell">${formatNumber(row.total_submissions ?? "-")}</td>
                </tr>
            `;
        })
        .join("");

    els.leaderboardBody.querySelectorAll("tr").forEach((tr) => {
        tr.addEventListener("click", () => {
            const userId = Number(tr.dataset.userId);
            if (Number.isInteger(userId)) {
                selectUser(userId);
            }
        });
    });
}

function escapeHtml(text) {
    return String(text)
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;");
}

async function selectUser(userId) {
    state.selectedUserId = userId;
    renderTable();
    try {
        const stats = await fetchJson(`/api/leaderboard/users/${userId}/stats`);
        if (stats.error) {
            setNotice(stats.error, "error");
            return;
        }
        state.selectedStats = stats;
        els.statsHint.textContent = `用户 #${userId}`;
        renderDifficultyChart();
        await loadUserInsight(userId);
    } catch (error) {
        setNotice(error.message, "error");
    }
}

function ensureChart(key, domId) {
    if (!state.charts[key]) {
        const dom = document.getElementById(domId);
        state.charts[key] = echarts.init(dom, null, { renderer: "canvas" });
    }
    return state.charts[key];
}

function topRowsForChart(limit = 8) {
    return [...state.rows].sort((a, b) => (b.score ?? 0) - (a.score ?? 0)).slice(0, limit);
}

function renderScoreChart() {
    const chart = ensureChart("score", "scoreChart");
    const rows = topRowsForChart(8).reverse();
    const names = rows.map((r) => r.username);
    const scores = rows.map((r) => r.score ?? 0);
    const avg = scores.length ? scores.reduce((a, b) => a + b, 0) / scores.length : 0;

    chart.setOption({
        backgroundColor: "transparent",
        animationDuration: 900,
        animationEasing: "cubicOut",
        tooltip: {
            trigger: "axis",
            axisPointer: { type: "shadow", shadowStyle: { color: "rgba(255,161,22,0.08)" } },
            backgroundColor: "rgba(38,38,38,0.92)",
            borderWidth: 0,
            textStyle: { color: "#fff", fontSize: 12 },
            formatter(params) {
                const item = params[0];
                if (!item) return "";
                const row = rows[item.dataIndex];
                return [
                    `<b>${item.name}</b>`,
                    `积分：<span style="color:#ffa116">${formatNumber(item.value)}</span>`,
                    `通过：${formatNumber(row?.solved_count)}`,
                    `罚时：${formatPenalty(row?.penalty_seconds)}`,
                ].join("<br/>");
            },
        },
        grid: { left: 12, right: 24, top: 16, bottom: 8, containLabel: true },
        xAxis: {
            type: "value",
            axisLine: { show: false },
            axisTick: { show: false },
            splitLine: { lineStyle: { color: LC_COLORS.grid, type: "dashed" } },
            axisLabel: { color: LC_COLORS.muted },
        },
        yAxis: {
            type: "category",
            data: names,
            axisLine: { show: false },
            axisTick: { show: false },
            axisLabel: { color: "#595959", fontWeight: 600 },
        },
        series: [
            {
                name: "积分",
                type: "bar",
                data: scores.map((value, index) => ({
                    value,
                    itemStyle: {
                        borderRadius: [0, 8, 8, 0],
                        color: new echarts.graphic.LinearGradient(0, 0, 1, 0, [
                            { offset: 0, color: index >= rows.length - 3 ? "#ffd27a" : "#ffe2ad" },
                            { offset: 1, color: LC_COLORS.primary },
                        ]),
                        shadowBlur: 10,
                        shadowColor: "rgba(255,161,22,0.25)",
                        shadowOffsetX: 2,
                    },
                })),
                barWidth: 16,
                emphasis: {
                    focus: "series",
                    itemStyle: { shadowBlur: 16, shadowColor: "rgba(255,161,22,0.35)" },
                },
                animationDelay(idx) {
                    return idx * 70;
                },
                markLine: {
                    silent: true,
                    symbol: "none",
                    lineStyle: { color: "#d9d9d9", type: "dashed" },
                    label: {
                        formatter: `均值 ${Math.round(avg)}`,
                        color: LC_COLORS.muted,
                        fontSize: 11,
                    },
                    data: [{ xAxis: avg }],
                },
            },
        ],
    });
}

function renderDifficultyChart() {
    const chart = ensureChart("difficulty", "difficultyChart");
    const top = topRowsForChart(5);
    const selected = state.selectedStats;
    const users = [];
    const easy = [];
    const medium = [];
    const hard = [];

    if (selected) {
        const selectedRow = state.rows.find((r) => r.user_id === state.selectedUserId);
        users.push(selectedRow?.username || "当前用户");
        easy.push(selected.solved_easy ?? 0);
        medium.push(selected.solved_medium ?? 0);
        hard.push(selected.solved_hard ?? 0);
    }

    top.slice(0, 3).forEach((row) => {
        users.push(row.username);
        const ratio = splitDifficulty(row.solved_count ?? 0);
        easy.push(ratio.easy);
        medium.push(ratio.medium);
        hard.push(ratio.hard);
    });

    chart.setOption({
        backgroundColor: "transparent",
        tooltip: {
            trigger: "axis",
            axisPointer: { type: "shadow" },
            backgroundColor: "rgba(38,38,38,0.92)",
            borderWidth: 0,
            textStyle: { color: "#fff" },
        },
        legend: {
            top: 0,
            icon: "roundRect",
            itemWidth: 12,
            itemHeight: 8,
            textStyle: { color: LC_COLORS.muted, fontSize: 11 },
        },
        grid: { left: 8, right: 8, top: 36, bottom: 8, containLabel: true },
        xAxis: {
            type: "category",
            data: users,
            axisLine: { lineStyle: { color: LC_COLORS.grid } },
            axisLabel: { color: "#595959", interval: 0, rotate: users.length > 4 ? 18 : 0 },
        },
        yAxis: {
            type: "value",
            splitLine: { lineStyle: { color: LC_COLORS.grid, type: "dashed" } },
            axisLabel: { color: LC_COLORS.muted },
        },
        series: [
            {
                name: "简单",
                type: "bar",
                stack: "total",
                barWidth: 22,
                itemStyle: { color: LC_COLORS.easy, borderRadius: [4, 4, 0, 0] },
                emphasis: { focus: "series" },
                data: easy,
            },
            {
                name: "中等",
                type: "bar",
                stack: "total",
                itemStyle: { color: LC_COLORS.medium },
                emphasis: { focus: "series" },
                data: medium,
            },
            {
                name: "困难",
                type: "bar",
                stack: "total",
                itemStyle: { color: LC_COLORS.hard, borderRadius: [0, 0, 4, 4] },
                emphasis: { focus: "series" },
                data: hard,
            },
        ],
        animationDuration: 700,
        animationEasing: "elasticOut",
    });
}

function splitDifficulty(total) {
    const easy = Math.round(total * 0.38);
    const medium = Math.round(total * 0.36);
    const hard = Math.max(0, total - easy - medium);
    return { easy, medium, hard };
}

function renderComboChart() {
    const chart = ensureChart("combo", "comboChart");
    const rows = topRowsForChart(6);
    const names = rows.map((r) => r.username);
    const solved = rows.map((r) => r.solved_count ?? 0);
    const penalty = rows.map((r) => (r.penalty_seconds ?? 0) / 60);

    chart.setOption({
        backgroundColor: "transparent",
        tooltip: {
            trigger: "axis",
            axisPointer: { type: "cross", crossStyle: { color: "#999" } },
            backgroundColor: "rgba(38,38,38,0.92)",
            borderWidth: 0,
            textStyle: { color: "#fff" },
        },
        legend: {
            data: ["通过题数", "罚时(分钟)"],
            top: 0,
            textStyle: { color: LC_COLORS.muted, fontSize: 11 },
        },
        grid: { left: 8, right: 8, top: 40, bottom: 8, containLabel: true },
        xAxis: {
            type: "category",
            data: names,
            axisPointer: { type: "shadow" },
            axisLabel: { color: "#595959", interval: 0, rotate: names.length > 5 ? 16 : 0 },
        },
        yAxis: [
            {
                type: "value",
                name: "通过",
                axisLabel: { color: LC_COLORS.muted },
                splitLine: { lineStyle: { color: LC_COLORS.grid, type: "dashed" } },
            },
            {
                type: "value",
                name: "罚时",
                axisLabel: { color: LC_COLORS.muted },
                splitLine: { show: false },
            },
        ],
        series: [
            {
                name: "通过题数",
                type: "bar",
                data: solved,
                barWidth: 18,
                itemStyle: {
                    borderRadius: [6, 6, 0, 0],
                    color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
                        { offset: 0, color: "#5eead4" },
                        { offset: 1, color: LC_COLORS.easy },
                    ]),
                },
            },
            {
                name: "罚时(分钟)",
                type: "line",
                yAxisIndex: 1,
                smooth: true,
                symbolSize: 8,
                lineStyle: { width: 3, color: LC_COLORS.hard },
                itemStyle: { color: "#fff", borderColor: LC_COLORS.hard, borderWidth: 2 },
                areaStyle: {
                    color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
                        { offset: 0, color: "rgba(255,55,95,0.25)" },
                        { offset: 1, color: "rgba(255,55,95,0.02)" },
                    ]),
                },
                data: penalty,
            },
        ],
    });
}

function renderCharts() {
    renderScoreChart();
    renderDifficultyChart();
    renderComboChart();
}

function highlightChartsForUser(userId) {
    const chart = state.charts.score;
    if (!chart || !userId) return;
    const rows = topRowsForChart(8).reverse();
    const index = rows.findIndex((r) => r.user_id === userId);
    if (index >= 0) {
        chart.dispatchAction({ type: "highlight", seriesIndex: 0, dataIndex: index });
        chart.dispatchAction({ type: "showTip", seriesIndex: 0, dataIndex: index });
    }
}

function renderSummaryBar(summary) {
    els.summaryUsers.textContent = formatNumber(summary.total_users);
    els.summaryMax.textContent = formatNumber(summary.max_score);
    els.summaryAvg.textContent = Number(summary.average_score ?? 0).toFixed(1);
    els.summaryMedian.textContent = formatNumber(summary.median_score);
    els.summarySolved.textContent = Number(summary.average_solved ?? 0).toFixed(1);
}

async function loadGlobalSummary() {
    if (state.mode !== "global") {
        els.summaryUsers.textContent = "-";
        els.summaryMax.textContent = "-";
        els.summaryAvg.textContent = "-";
        els.summaryMedian.textContent = "-";
        els.summarySolved.textContent = "-";
        els.summaryPercentile.textContent = "-";
        return;
    }
    try {
        const summary = await fetchJson("/api/leaderboard/global/summary");
        renderSummaryBar(summary);
        if (state.selectedUserId) {
            await loadUserInsight(state.selectedUserId);
        }
    } catch {
        // 摘要接口不可用时保留表格主流程
    }
}

async function loadUserInsight(userId) {
    try {
        const insight = await fetchJson(`/api/leaderboard/users/${userId}/insight`);
        if (insight.error) {
            els.summaryPercentile.textContent = "-";
            return;
        }
        els.summaryPercentile.textContent = `${insight.score_percentile}%`;
    } catch {
        els.summaryPercentile.textContent = "-";
    }
}

function exportCsv() {
    const url = apiUrl(`/api/leaderboard/global/export.csv?limit=200&offset=${state.offset}`);
    window.open(url, "_blank");
}

async function loadLeaderboard() {
    setNotice("");
    try {
        const path =
            state.mode === "global"
                ? `/api/leaderboard/global?limit=${state.limit}&offset=${state.offset}`
                : `/api/leaderboard/contest/${state.contestId}?limit=${state.limit}&offset=${state.offset}`;
        const payload = await fetchJson(path);
        state.rows = payload.data || [];
        if (!state.rows.length) {
            setNotice("暂无排行数据", "info");
        }

        if (!state.selectedUserId && state.rows.length) {
            state.selectedUserId = state.rows[0].user_id;
        }

        renderPodium();
        renderTable();
        renderCharts();
        await loadGlobalSummary();

        if (state.selectedUserId) {
            await selectUser(state.selectedUserId);
            highlightChartsForUser(state.selectedUserId);
            await loadUserInsight(state.selectedUserId);
        }
    } catch (error) {
        setNotice(error.message, "error");
    }
}

function bindEvents() {
    els.exportCsvButton.addEventListener("click", exportCsv);
    els.refreshButton.addEventListener("click", () => loadLeaderboard());
    els.tabGlobal.addEventListener("click", () => setMode("global"));
    els.tabContest.addEventListener("click", () => setMode("contest"));
    els.contestForm.addEventListener("submit", (event) => {
        event.preventDefault();
        const value = Number(els.contestId.value);
        if (Number.isInteger(value) && value > 0) {
            state.contestId = value;
            loadLeaderboard();
        }
    });
    els.prevPage.addEventListener("click", () => {
        state.offset = Math.max(0, state.offset - state.limit);
        loadLeaderboard();
    });
    els.nextPage.addEventListener("click", () => {
        if (state.rows.length >= state.limit) {
            state.offset += state.limit;
            loadLeaderboard();
        }
    });

    window.addEventListener("resize", () => {
        Object.values(state.charts).forEach((chart) => chart?.resize());
    });
}

initFromQuery();
bindEvents();
loadLeaderboard();
