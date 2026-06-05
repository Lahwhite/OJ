package com.oj.problem.repository;

import com.oj.problem.entity.ProblemUserStatusEntity;
import java.util.List;
import java.util.Optional;
import org.springframework.data.jpa.repository.JpaRepository;

public interface ProblemUserStatusRepository extends JpaRepository<ProblemUserStatusEntity, Long> {

    Optional<ProblemUserStatusEntity> findByUserIdAndProblem_Id(Long userId, Long problemId);

    List<ProblemUserStatusEntity> findByUserIdOrderByProblem_IdAsc(Long userId);
}
