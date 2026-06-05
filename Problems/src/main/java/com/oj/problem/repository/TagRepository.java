// 题目模块：该文件负责具体的数据结构、接口或业务逻辑
package com.oj.problem.repository;

import com.oj.problem.entity.TagEntity;
import java.util.Collection;
import java.util.List;
import org.springframework.data.jpa.repository.JpaRepository;

// 对外暴露的方法或字段，通常承接模块间协作
public interface TagRepository extends JpaRepository<TagEntity, Long> {

    // 批量按名字查，创建/更新题目时用来同步标签
    List<TagEntity> findByNameIn(Collection<String> names);
}
