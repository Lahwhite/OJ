const VERDICT_LABELS = {
  AC: "Accepted",
  WA: "Wrong Answer",
  TLE: "Time Limit Exceeded",
  MLE: "Memory Limit Exceeded",
  RE: "Runtime Error",
  CE: "Compile Error",
  PENDING: "Pending",
  JUDGING: "Judging",
  UNKNOWN: "Unknown",
  ERROR: "Error",
};

const STATUS_NAMES = [
  "PENDING",
  "JUDGING",
  "ACCEPTED",
  "WRONG_ANSWER",
  "TIME_LIMIT_EXCEEDED",
  "MEMORY_LIMIT_EXCEEDED",
  "RUNTIME_ERROR",
  "COMPILE_ERROR",
];

const $ = (id) => document.getElementById(id);

function showParseError(msg) {
  const el = $("parse-error");
  if (msg) {
    el.textContent = msg;
    el.classList.remove("hidden");
  } else {
    el.textContent = "";
    el.classList.add("hidden");
  }
}

function formatMemory(kb) {
  if (kb == null || Number.isNaN(kb)) return "—";
  if (kb >= 1024) return `${(kb / 1024).toFixed(2)} MB`;
  return `${kb} KB`;
}

function formatRuntime(ms) {
  if (ms == null || Number.isNaN(ms)) return "—";
  return `${ms} ms`;
}

function normalizeVerdict(v) {
  if (!v) return "UNKNOWN";
  const s = String(v).toUpperCase();
  if (VERDICT_LABELS[s]) return s;
  return "UNKNOWN";
}

function verdictFromStatus(statusInt) {
  const map = ["PENDING", "JUDGING", "AC", "WA", "TLE", "MLE", "RE", "CE"];
  if (typeof statusInt === "number" && statusInt >= 0 && statusInt < map.length) {
    return map[statusInt];
  }
  return null;
}

function normalizeJudgePayload(raw) {
  if (raw.error) {
    return {
      type: "error",
      data: {
        verdict: "ERROR",
        error_message: raw.message || raw.error_message || String(raw.error),
        raw,
      },
    };
  }

  const verdict =
    normalizeVerdict(raw.verdict) ||
    verdictFromStatus(raw.overall_status) ||
    "UNKNOWN";

  const testCases = Array.isArray(raw.test_cases) ? raw.test_cases : [];

  return {
    type: "result",
    data: {
      program_language: raw.program_language || "",
      overall_status: raw.overall_status,
      verdict,
      runtime_ms: raw.runtime ?? raw.runtime_ms,
      memory_kb: raw.memory ?? raw.memory_kb,
      error_message: raw.error_message || "",
      total_score: raw.total_score ?? 0,
      max_score: raw.max_score ?? 0,
      test_cases: testCases.map((tc, i) => ({
        case_id: tc.case_id ?? tc.id ?? i + 1,
        status: tc.status,
        verdict:
          normalizeVerdict(tc.verdict) ||
          verdictFromStatus(tc.status) ||
          "UNKNOWN",
        passed: Boolean(tc.passed),
        runtime_ms: tc.runtime ?? tc.runtime_ms ?? 0,
        memory_kb: tc.memory ?? tc.memory_kb ?? 0,
        score: tc.score ?? tc.score_awarded ?? 0,
      })),
      raw,
    },
  };
}

function renderResult(payload) {
  const panel = $("result-panel");
  panel.classList.remove("hidden");

  if (payload.type === "error") {
    renderErrorOnly(payload.data);
    return;
  }

  const d = payload.data;
  const verdict = d.verdict;

  const heroVerdict = $("hero-verdict");
  heroVerdict.textContent = verdict;
  heroVerdict.className = `hero__verdict verdict--${verdict}`;
  heroVerdict.title = VERDICT_LABELS[verdict] || verdict;

  const langEl = $("hero-lang");
  langEl.textContent = d.program_language
    ? `语言：${d.program_language}`
    : "";
  langEl.classList.toggle("hidden", !d.program_language);

  const statusEl = $("hero-status");
  if (d.overall_status != null) {
    const name = STATUS_NAMES[d.overall_status] || `#${d.overall_status}`;
    statusEl.textContent = `status = ${d.overall_status} (${name})`;
    statusEl.classList.remove("hidden");
  } else {
    statusEl.classList.add("hidden");
  }

  const errEl = $("hero-error");
  if (d.error_message) {
    errEl.textContent = d.error_message;
    errEl.classList.remove("hidden");
  } else {
    errEl.classList.add("hidden");
  }

  const maxScore = d.max_score > 0 ? d.max_score : 1;
  const pct = Math.min(100, Math.round((d.total_score / maxScore) * 100));
  $("stat-score").textContent =
    d.max_score > 0 ? `${d.total_score} / ${d.max_score}` : String(d.total_score);
  $("score-bar").style.width = `${pct}%`;

  $("stat-runtime").textContent = formatRuntime(d.runtime_ms);
  $("stat-memory").textContent = formatMemory(d.memory_kb);

  const passed = d.test_cases.filter((t) => t.passed).length;
  const total = d.test_cases.length;
  $("stat-passed").textContent =
    total > 0 ? `${passed} / ${total}` : "—";

  renderTestCases(d.test_cases);
  $("raw-json").textContent = JSON.stringify(d.raw, null, 2);

  panel.scrollIntoView({ behavior: "smooth", block: "start" });
}

function renderErrorOnly(data) {
  const heroVerdict = $("hero-verdict");
  heroVerdict.textContent = "ERROR";
  heroVerdict.className = "hero__verdict verdict--ERROR";
  heroVerdict.title = "CLI / 解析错误";

  $("hero-lang").classList.add("hidden");
  $("hero-status").classList.add("hidden");

  const errEl = $("hero-error");
  errEl.textContent = data.error_message;
  errEl.classList.remove("hidden");

  $("stat-score").textContent = "—";
  $("score-bar").style.width = "0%";
  $("stat-runtime").textContent = "—";
  $("stat-memory").textContent = "—";
  $("stat-passed").textContent = "—";

  $("cases-section").classList.add("hidden");
  $("raw-json").textContent = JSON.stringify(data.raw, null, 2);
}

function renderTestCases(cases) {
  $("cases-section").classList.remove("hidden");
  $("cases-count").textContent =
    cases.length > 0 ? `共 ${cases.length} 个` : "无测试用例";

  const grid = $("cases-grid");
  grid.innerHTML = "";
  cases.forEach((tc) => {
    const chip = document.createElement("div");
    chip.className = `case-chip case-chip--${tc.verdict}`;
    chip.title = `${VERDICT_LABELS[tc.verdict] || tc.verdict}\n${formatRuntime(tc.runtime_ms)}, ${formatMemory(tc.memory_kb)}`;
    chip.innerHTML = `
      <span class="case-chip__id">#${tc.case_id}</span>
      <span class="case-chip__verdict">${tc.verdict}</span>
    `;
    grid.appendChild(chip);
  });

  const table = $("cases-table");
  const tbody = $("cases-tbody");
  if (cases.length === 0) {
    table.classList.add("hidden");
    return;
  }
  table.classList.remove("hidden");
  tbody.innerHTML = cases
    .map(
      (tc) => `
    <tr>
      <td>${tc.case_id}</td>
      <td><span class="badge badge--${tc.verdict}">${tc.verdict}</span></td>
      <td class="${tc.passed ? "pass-yes" : "pass-no"}">${tc.passed ? "是" : "否"}</td>
      <td>${formatRuntime(tc.runtime_ms)}</td>
      <td>${formatMemory(tc.memory_kb)}</td>
      <td>${tc.score}</td>
    </tr>
  `
    )
    .join("");
}

function handleJsonText(text) {
  showParseError("");
  let parsed;
  try {
    parsed = JSON.parse(text);
  } catch (e) {
    showParseError(`JSON 解析失败：${e.message}`);
    return;
  }
  if (typeof parsed !== "object" || parsed === null) {
    showParseError("根节点必须是 JSON 对象");
    return;
  }
  renderResult(normalizeJudgePayload(parsed));
}

function readFileAsText(file) {
  const reader = new FileReader();
  reader.onload = () => {
    $("json-paste").value = reader.result;
    handleJsonText(reader.result);
  };
  reader.onerror = () => showParseError("读取文件失败");
  reader.readAsText(file, "UTF-8");
}

function initDropzone() {
  const zone = $("dropzone");
  const input = $("file-input");

  zone.addEventListener("click", () => input.click());
  zone.addEventListener("keydown", (e) => {
    if (e.key === "Enter" || e.key === " ") {
      e.preventDefault();
      input.click();
    }
  });

  input.addEventListener("change", () => {
    const f = input.files?.[0];
    if (f) readFileAsText(f);
    input.value = "";
  });

  ["dragenter", "dragover"].forEach((ev) => {
    zone.addEventListener(ev, (e) => {
      e.preventDefault();
      zone.classList.add("dragover");
    });
  });
  ["dragleave", "drop"].forEach((ev) => {
    zone.addEventListener(ev, (e) => {
      e.preventDefault();
      zone.classList.remove("dragover");
    });
  });
  zone.addEventListener("drop", (e) => {
    const f = e.dataTransfer?.files?.[0];
    if (f) readFileAsText(f);
  });
}

async function loadLatestResult() {
  const params = new URLSearchParams(location.search);
  const cacheBust = params.get("t") || Date.now();
  const resultUrl = params.get("result_url");
  const target = resultUrl
    ? `${resultUrl}${resultUrl.includes("?") ? "&" : "?"}t=${encodeURIComponent(cacheBust)}`
    : `latest_result.json?t=${encodeURIComponent(cacheBust)}`;
  try {
    const res = await fetch(target, { cache: "no-store" });
    if (!res.ok) throw new Error(`${res.status} ${res.statusText}`);
    const text = await res.text();
    $("json-paste").value = text;
    handleJsonText(text);
    $("input-panel").classList.add("hidden");
  } catch (e) {
    showParseError(`自动加载评测结果失败：${e.message}`);
  }
}

function init() {
  initDropzone();

  $("btn-parse").addEventListener("click", () => {
    const text = $("json-paste").value.trim();
    if (!text) {
      showParseError("请先粘贴或加载 JSON");
      return;
    }
    handleJsonText(text);
  });

  $("btn-clear").addEventListener("click", () => {
    $("json-paste").value = "";
    showParseError("");
    $("result-panel").classList.add("hidden");
  });

  const params = new URLSearchParams(location.search);
  if (params.has("latest")) {
    loadLatestResult();
  }
}

document.addEventListener("DOMContentLoaded", init);
