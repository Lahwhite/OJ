package com.oj.problem.dto.response;

public class SubmitCodeResponse {

    // 提交人用户名，会用于评测临时文件名前缀和用户状态记录。
    private String username;
    // 题目 ID，用于定位本次提交或状态更新关联的题目。
    private Long problemId;
    // 提交语言标识，必须能映射到评测引擎支持的源文件名。
    private String language;
    // 提交接口给前端展示的结果摘要。
    private String message;
    // 评测引擎输出的本地结果文件路径。
    private String resultFile;
    // 可在浏览器打开的评测报告链接。
    private String resultUrl;
    // 用户模块传入的用户 ID，用于区分不同用户的做题状态。
    private Long userId;
    // 是否已经通过该题，历史上任意一次 AC 后保持为 true。
    private Boolean accepted;
    // 单个测试点分值，评测结果会累加为 total_score。
    private Integer score;
    // 本题当前评测总分，和 score 一起展示分数占比。
    private Integer maxScore;
    private String verdict;

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    public Long getProblemId() {
        return problemId;
    }

    public void setProblemId(Long problemId) {
        this.problemId = problemId;
    }

    public String getLanguage() {
        return language;
    }

    public void setLanguage(String language) {
        this.language = language;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public String getResultFile() {
        return resultFile;
    }

    public void setResultFile(String resultFile) {
        this.resultFile = resultFile;
    }

    public String getResultUrl() {
        return resultUrl;
    }

    public void setResultUrl(String resultUrl) {
        this.resultUrl = resultUrl;
    }

    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }

    public Boolean getAccepted() {
        return accepted;
    }

    public void setAccepted(Boolean accepted) {
        this.accepted = accepted;
    }

    public Integer getScore() {
        return score;
    }

    public void setScore(Integer score) {
        this.score = score;
    }

    public Integer getMaxScore() {
        return maxScore;
    }

    public void setMaxScore(Integer maxScore) {
        this.maxScore = maxScore;
    }

    public String getVerdict() {
        return verdict;
    }

    public void setVerdict(String verdict) {
        this.verdict = verdict;
    }
}
