package com.oj.problem.dto.response;

import java.util.List;

public class ProblemStatusListResponse {

    // 查询某个用户所有题目的做题状态
    private Long userId;
    private List<ProblemStatusResponse> statuses;

    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }

    public List<ProblemStatusResponse> getStatuses() {
        return statuses;
    }

    public void setStatuses(List<ProblemStatusResponse> statuses) {
        this.statuses = statuses;
    }
}
