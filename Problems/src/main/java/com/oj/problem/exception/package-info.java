/**
 * Exception and error-response mapping for the problem module.
 *
 * Business code throws {@code BusinessException} with a module-specific code and
 * HTTP status; the global handler turns those failures into the same
 * {@code ApiResponse} envelope used by successful endpoints. Validation errors
 * are flattened here as well, so controllers and services can focus on domain
 * rules instead of repeating response formatting.
 */
package com.oj.problem.exception;
