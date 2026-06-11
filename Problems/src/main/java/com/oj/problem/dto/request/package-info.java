/**
 * Request DTOs accepted by the problem module.
 *
 * These classes define the external write/query contract and carry validation
 * annotations close to the fields that enter the system. Services may still
 * normalize values, such as trimming tags or clamping pagination, but they
 * should not need to reinterpret raw HTTP details. Keeping request objects
 * separate from JPA entities also prevents clients from writing persistence-only
 * fields such as counters and timestamps.
 */
package com.oj.problem.dto.request;
