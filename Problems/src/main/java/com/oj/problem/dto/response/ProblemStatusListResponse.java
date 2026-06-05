// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.dto.response;

import java.util.List;

// 对外暴露的方法或字段，通常承接模块间协作
public class ProblemStatusListResponse {

    // 查询某个用户所有题目的做题状态
    private Long userId;
    private List<ProblemStatusResponse> statuses;

    // 对外暴露的方法或字段，通常承接模块间协作
    public Long getUserId() {
        // 返回本阶段计算结果，供上层流程继续使用
        return userId;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setUserId(Long userId) {
        this.userId = userId;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public List<ProblemStatusResponse> getStatuses() {
        // 返回本阶段计算结果，供上层流程继续使用
        return statuses;
    }

    // 对外暴露的方法或字段，通常承接模块间协作
    public void setStatuses(List<ProblemStatusResponse> statuses) {
        this.statuses = statuses;
    }
}
