package org.example.users.repository;

import org.example.users.entity.ProblemUser;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface ProblemUserRepository extends JpaRepository<ProblemUser, Long> {
    ProblemUser findByUsername(String username);
}