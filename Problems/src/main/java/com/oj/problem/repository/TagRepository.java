package com.oj.problem.repository;

import com.oj.problem.entity.TagEntity;
import java.util.Collection;
import java.util.List;
import org.springframework.data.jpa.repository.JpaRepository;

public interface TagRepository extends JpaRepository<TagEntity, Long> {

    // 批量按名字查，创建/更新题目时用来同步标签
    List<TagEntity> findByNameIn(Collection<String> names);
}
