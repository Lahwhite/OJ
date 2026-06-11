/**
 * Configuration objects for external module integration.
 *
 * The problem module delegates real code execution to the judge engine, so paths
 * and timeouts are bound from configuration instead of hard-coded in services.
 * Keeping these settings typed makes startup behavior easier to inspect and
 * gives tests a single place to override judge-related values.
 */
package com.oj.problem.config;
