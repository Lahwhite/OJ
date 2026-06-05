// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.repository;

import com.oj.problem.entity.ProblemUserStatusEntity;
import java.util.List;
import java.util.Optional;
import org.springframework.data.jpa.repository.JpaRepository;

// 对外暴露的方法或字段，通常承接模块间协作
public interface ProblemUserStatusRepository extends JpaRepository<ProblemUserStatusEntity, Long> {

    // 查单条用于更新时判断是否已有记录
    Optional<ProblemUserStatusEntity> findByUserIdAndProblem_Id(Long userId, Long problemId);

    // 查用户所有题目状态，按题目 id 排序方便前端展示
    List<ProblemUserStatusEntity> findByUserIdOrderByProblem_IdAsc(Long userId);
}
