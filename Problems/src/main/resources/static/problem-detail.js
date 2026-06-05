const API_BASE = new URL("./v1/problems", window.location.href).pathname.replace(/\/$/, "");

const DEFAULT_CODE = {
    cpp: `#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    
    // 在此编写你的解题代码
    
    return 0;
}`,
    java: `import java.util.*;

public class Main {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        
        // 在此编写你的解题代码
        
    }
}`,
    python: `import sys

def main():
    # 在此编写你的解题代码
    pass

if __name__ == "__main__":
    main()`,
    c: `#include <stdio.h>

int main() {
    // 在此编写你的解题代码
    
    return 0;
}`,
    javascript: `const readline = require('readline');
const rl = readline.createInterface({ input: process.stdin });
const lines = [];
rl.on('line', line => lines.push(line.trim()));
rl.on('close', () => {
    // 在此编写你的解题代码
    
});`
};

const MONACO_LANGUAGE = {
    cpp: "cpp",
    java: "java", 
    python: "python",
    c: "c",
    javascript: "javascript"
};

const state = {
    problem: null,
    editor: null,
    currentLanguage: "cpp",
    userCode: {}
};

const els = {
    notice: document.querySelector("#notice"),
    homeLink: document.querySelector("#homeLink"),
    problemTitle: document.querySelector("#problemTitle"),
    problemContent: document.querySelector("#problemContent"),
    languageSelect: document.querySelector("#languageSelect"),
    resetButton: document.querySelector("#resetButton"),
    runButton: document.querySelector("#runButton"),
    submitButton: document.querySelector("#submitButton"),
    resultPanel: document.querySelector("#resultPanel"),
    resultContent: document.querySelector("#resultContent"),
    closeResult: document.querySelector("#closeResult"),
    timeLimit: document.querySelector("#timeLimit"),
    memoryLimit: document.querySelector("#memoryLimit")
};

function getParams() {
    const p = new URLSearchParams(window.location.search);
    return {
        id: p.get("id"),
        returnUrl: p.get("return_url"),
        userId: p.get("user_id") || p.get("userId")
    };
}

function setNotice(message, type = "info") {
    if (!message) {
        els.notice.className = "hidden border-b px-6 py-3 text-sm";
        els.notice.textContent = "";
        return;
    }
    const themes = {
        info: "border-blue-200 bg-blue-50 text-blue-700",
        success: "border-green-200 bg-green-50 text-green-700",
        error: "border-red-200 bg-red-50 text-red-700"
    };
    els.notice.className = `border-b px-6 py-3 text-sm ${themes[type] || themes.info}`;
    els.notice.textContent = message;
}

function escapeHtml(value) {
    return String(value ?? "")
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}

function difficultyBadge(difficulty) {
    const map = {
        easy: ["简单", "bg-green-50 text-green-700"],
        medium: ["中等", "bg-yellow-50 text-yellow-700"],
        hard: ["困难", "bg-red-50 text-red-700"]
    };
    const [label, cls] = map[difficulty] || [difficulty || "未知", "bg-gray-100 text-gray-600"];
    return `<span class="rounded-lg px-3 py-1 text-xs font-semibold ${cls}">${escapeHtml(label)}</span>`;
}

function tagBadges(tags = []) {
    return tags.map(t => 
        `<span class="rounded-lg bg-indigo-50 px-2 py-1 text-xs font-medium text-indigo-600">${escapeHtml(t)}</span>`
    ).join("");
}

function formatRate(rate) {
    return `${((Number(rate) || 0) * 100).toFixed(1)}%`;
}

async function loadProblem(id) {
    try {
        const res = await fetch(`${API_BASE}/${id}`);
        const text = await res.text();
        const body = text ? JSON.parse(text) : null;
        if (!res.ok || (body && body.code >= 400)) {
            throw new Error(body?.message || `HTTP ${res.status}`);
        }
        return body?.data ?? null;
    } catch (err) {
        setNotice(`加载题目失败：${err.message}`, "error");
        els.problemContent.innerHTML = 
            `<div class="py-20 text-center text-gray-400">${escapeHtml(err.message)}</div>`;
        return null;
    }
}

function renderHeader(problem) {
    els.problemTitle.innerHTML = `
        <span class="text-sm text-gray-400">#${problem.id}</span>
        <span class="font-semibold text-gray-800">${escapeHtml(problem.title)}</span>
        ${difficultyBadge(problem.difficulty)}
    `;
    document.title = `P${problem.id} · ${problem.title} - OJ`;
    els.timeLimit.textContent = `${problem.timeLimit ?? "-"} ms`;
    els.memoryLimit.textContent = `${problem.memoryLimit ?? "-"} MB`;
}

function renderContent(problem) {
    const tagHtml = tagBadges(problem.tags || []);
    const hasSample = problem.sampleInput || problem.sampleOutput;
    
    els.problemContent.innerHTML = `
        <div class="flex flex-wrap items-center gap-2">
            ${difficultyBadge(problem.difficulty)}
            ${tagHtml}
        </div>

        <section>
            <h2 class="text-lg font-bold text-gray-800">题目描述</h2>
            <p class="mt-3 whitespace-pre-wrap leading-7 text-gray-700">${escapeHtml(problem.description || "")}</p>
        </section>

        ${problem.inputDescription ? `
        <section>
            <h2 class="text-lg font-bold text-gray-800">输入说明</h2>
            <p class="mt-3 whitespace-pre-wrap leading-7 text-gray-700">${escapeHtml(problem.inputDescription)}</p>
        </section>` : ""}

        ${problem.outputDescription ? `
        <section>
            <h2 class="text-lg font-bold text-gray-800">输出说明</h2>
            <p class="mt-3 whitespace-pre-wrap leading-7 text-gray-700">${escapeHtml(problem.outputDescription)}</p>
        </section>` : ""}

        ${hasSample ? `
        <section>
            <h2 class="text-lg font-bold text-gray-800">样例</h2>
            <div class="mt-3 grid gap-4 md:grid-cols-2">
                <div>
                    <div class="mb-2 text-sm font-medium text-gray-500">输入</div>
                    <pre class="overflow-x-auto rounded-xl bg-gray-900 p-4 text-sm leading-6 text-gray-100">${escapeHtml(problem.sampleInput || "")}</pre>
                </div>
                <div>
                    <div class="mb-2 text-sm font-medium text-gray-500">输出</div>
                    <pre class="overflow-x-auto rounded-xl bg-gray-900 p-4 text-sm leading-6 text-gray-100">${escapeHtml(problem.sampleOutput || "")}</pre>
                </div>
            </div>
        </section>` : ""}

        <section class="rounded-xl bg-gray-50 p-4 text-sm">
            <div class="grid grid-cols-3 gap-4">
                <div>
                    <div class="text-gray-500">时间限制</div>
                    <div class="mt-1 font-semibold text-gray-800">${problem.timeLimit ?? "-"} ms</div>
                </div>
                <div>
                    <div class="text-gray-500">内存限制</div>
                    <div class="mt-1 font-semibold text-gray-800">${problem.memoryLimit ?? "-"} MB</div>
                </div>
                <div>
                    <div class="text-gray-500">通过率</div>
                    <div class="mt-1 font-semibold text-gray-800">${formatRate(problem.passRate)} (${problem.acceptedCount || 0}/${problem.submissionCount || 0})</div>
                </div>
            </div>
        </section>
    `;
}

function initMonaco(language) {
    require.config({ 
        paths: { vs: "https://cdn.jsdelivr.net/npm/monaco-editor@0.45.0/min/vs" }
    });
    require(["vs/editor/editor.main"], () => {
        state.editor = monaco.editor.create(document.querySelector("#monacoEditor"), {
            value: state.userCode[language] ?? DEFAULT_CODE[language] ?? "",
            language: MONACO_LANGUAGE[language] || "plaintext",
            theme: "vs-dark",
            fontSize: 14,
            lineHeight: 22,
            fontFamily: "'Cascadia Code', 'JetBrains Mono', Menlo, Monaco, Consolas, monospace",
            minimap: { enabled: false },
            scrollBeyondLastLine: false,
            automaticLayout: true,
            tabSize: 4,
            wordWrap: "off",
            renderLineHighlight: "all",
            cursorBlinking: "smooth",
            smoothScrolling: true,
            padding: { top: 16, bottom: 16 }
        });
        
        state.editor.onDidChangeModelContent(() => {
            state.userCode[state.currentLanguage] = state.editor.getValue();
        });
    });
}

function switchLanguage(lang) {
    if (!state.editor) return;
    state.userCode[state.currentLanguage] = state.editor.getValue();
    state.currentLanguage = lang;
    monaco.editor.setModelLanguage(state.editor.getModel(), MONACO_LANGUAGE[lang] || "plaintext");
    state.editor.setValue(state.userCode[lang] ?? DEFAULT_CODE[lang] ?? "");
}

function resetCode() {
    if (!state.editor) return;
    if (!window.confirm("确定重置代码？当前编辑内容将丢失。")) return;
    delete state.userCode[state.currentLanguage];
    state.editor.setValue(DEFAULT_CODE[state.currentLanguage] || "");
}

function showResult(html) {
    els.resultContent.innerHTML = html;
    els.resultPanel.classList.remove("hidden");
}

function hideResult() {
    els.resultPanel.classList.add("hidden");
}

function handleRun() {
    if (!state.editor) return;
    const code = state.editor.getValue().trim();
    if (!code) {
        setNotice("请先编写代码。", "error");
        return;
    }
    
    const si = state.problem?.sampleInput || "";
    const so = state.problem?.sampleOutput || "";
    
    showResult(`
        <div class="space-y-3 text-sm">
            <div class="flex items-center gap-2">
                <span class="font-semibold text-gray-700">样例测试</span>
                <span class="rounded-full bg-yellow-100 px-3 py-0.5 text-xs font-medium text-yellow-700">演示模式</span>
            </div>
            <div class="grid gap-3 md:grid-cols-2">
                <div class="rounded-lg border border-gray-200 bg-white p-3">
                    <div class="mb-1 text-xs font-medium text-gray-500">输入</div>
                    <pre class="whitespace-pre-wrap font-mono text-xs text-gray-800">${escapeHtml(si || "(无样例)")}</pre>
                </div>
                <div class="rounded-lg border border-gray-200 bg-white p-3">
                    <div class="mb-1 text-xs font-medium text-gray-500">期望输出</div>
                    <pre class="whitespace-pre-wrap font-mono text-xs text-gray-800">${escapeHtml(so || "(无样例)")}</pre>
                </div>
            </div>
            <p class="text-xs text-gray-400">当前为演示模式，评测引擎尚未接入，代码不会实际执行。</p>
        </div>
    `);
}

function handleSubmit() {
    if (!state.editor) return;
    const code = state.editor.getValue().trim();
    if (!code) {
        setNotice("请先编写代码。", "error");
        return;
    }
    
    showResult(`
        <div class="flex flex-wrap items-center gap-3 text-sm">
            <span class="rounded-full bg-blue-100 px-3 py-1 text-xs font-semibold text-blue-700">已提交</span>
            <span class="text-gray-500">语言：${escapeHtml(state.currentLanguage.toUpperCase())}</span>
            <span class="text-gray-500">代码长度：${code.length} 字节</span>
            <span class="text-xs text-gray-400">评测功能待接入。</span>
        </div>
    `);
}

function bindEvents() {
    els.languageSelect.addEventListener("change", () => switchLanguage(els.languageSelect.value));
    els.resetButton.addEventListener("click", resetCode);
    els.runButton.addEventListener("click", handleRun);
    els.submitButton.addEventListener("click", handleSubmit);
    els.closeResult.addEventListener("click", hideResult);
}

async function init() {
    const { id, returnUrl } = getParams();
    
    if (returnUrl) {
        try {
            const u = new URL(returnUrl, window.location.href);
            if (u.protocol === "http:" || u.protocol === "https:") {
                els.homeLink.href = returnUrl;
            }
        } catch {}
    }
    
    if (!id) {
        setNotice("未指定题目 ID，请从题库页面进入。", "error");
        els.problemContent.innerHTML = 
            `<div class="py-20 text-center text-gray-400">未指定题目</div>`;
        return;
    }
    
    const problem = await loadProblem(id);
    if (!problem) return;
    
    state.problem = problem;
    renderHeader(problem);
    renderContent(problem);
    bindEvents();
    initMonaco(state.currentLanguage);
}

init();
