package com.oj.problem.dto.request;

import javax.validation.constraints.NotNull;

public class SubmissionResultRequest {

    @NotNull(message = "是否通过不能为空")
    private Boolean accepted;

    public Boolean getAccepted() {
        return accepted;
    }

    public void setAccepted(Boolean accepted) {
        this.accepted = accepted;
    }
}
