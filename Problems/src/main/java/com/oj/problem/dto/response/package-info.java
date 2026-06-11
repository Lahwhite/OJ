/**
 * Response DTOs returned by the problem module.
 *
 * Each response type is shaped for a specific screen or workflow: list rows stay
 * compact, detail responses include full statements and test cases, and mutation
 * responses only return enough data for the frontend to refresh the changed
 * record. This boundary keeps lazy-loaded entities and internal persistence
 * fields out of JSON responses while preserving a stable API for the frontend.
 */
package com.oj.problem.dto.response;
