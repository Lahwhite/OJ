package com.oj.problem.dto.response;

import java.util.List;

public class ProblemStatusListResponse {

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
