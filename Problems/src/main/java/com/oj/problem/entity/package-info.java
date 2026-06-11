/**
 * JPA persistence model for problems, tags, users, statuses, and test cases.
 *
 * Entities in this package describe database ownership and cascade behavior,
 * not API shape. {@code ProblemEntity} is the aggregate root for test cases and
 * tags, while user status records are updated from judge results. Defaults and
 * timestamp columns are coordinated with {@code schema.sql}, so code changes
 * here should be checked against the database definition.
 */
package com.oj.problem.entity;
