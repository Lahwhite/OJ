package com.oj.problem.repository;

import com.oj.problem.entity.ProblemEntity;
import java.util.Optional;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

public interface ProblemRepository extends JpaRepository<ProblemEntity, Long>, JpaSpecificationExecutor<ProblemEntity> {

    // 这个查询同时把 testCases 和 tags 关联进来，避免 N+1
    @Query("select problem from ProblemEntity problem where problem.id = :id")
    Optional<ProblemEntity> findWithTestCasesAndTagsById(@Param("id") Long id);
}
