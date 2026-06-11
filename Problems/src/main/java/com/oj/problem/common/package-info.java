/**
 * Shared response primitives for the problem module.
 *
 * The frontend expects every JSON endpoint to return a consistent envelope with
 * a business code, message, and optional data payload. Keeping that convention
 * in a small common package makes controller responses predictable and lets the
 * JavaScript clients handle HTTP errors and business errors through one path.
 */
package com.oj.problem.common;
