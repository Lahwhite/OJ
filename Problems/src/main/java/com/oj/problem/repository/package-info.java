/**
 * Spring Data repositories for the problem module.
 *
 * Repository interfaces hide query details from services while still naming the
 * fetch strategy where it matters. Methods that load problems together with
 * test cases or tags are intentionally explicit because the frontend and judge
 * flows need those associations immediately. New queries should prefer
 * descriptive method names or documented JPQL over ad-hoc filtering in callers.
 */
package com.oj.problem.repository;
