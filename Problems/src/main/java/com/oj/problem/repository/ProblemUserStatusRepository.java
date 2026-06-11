package com.oj.problem.repository;

import com.oj.problem.entity.ProblemUserStatusEntity;
import java.util.List;
import java.util.Optional;
import org.springframework.data.jpa.repository.JpaRepository;

public interface ProblemUserStatusRepository extends JpaRepository<ProblemUserStatusEntity, Long> {

    // 查单条用于更新时判断是否已有记录
    Optional<ProblemUserStatusEntity> findByUserIdAndProblem_Id(Long userId, Long problemId);

    // 查用户所有题目状态，按题目 id 排序方便前端展示
    List<ProblemUserStatusEntity> findByUserIdOrderByProblem_IdAsc(Long userId);
}
