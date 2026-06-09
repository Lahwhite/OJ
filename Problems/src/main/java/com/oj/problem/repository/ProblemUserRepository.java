package com.oj.problem.repository;

import com.oj.problem.entity.ProblemUserEntity;
import java.util.Optional;
import org.springframework.data.jpa.repository.JpaRepository;

public interface ProblemUserRepository extends JpaRepository<ProblemUserEntity, Long> {

    Optional<ProblemUserEntity> findByUsername(String username);
}
