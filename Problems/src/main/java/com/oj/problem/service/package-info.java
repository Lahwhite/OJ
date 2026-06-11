/**
 * Service contracts used by controllers and cross-module entry points.
 *
 * Interfaces in this package describe the operations the rest of the module may
 * rely on without exposing persistence details. They separate problem CRUD,
 * judge submission, and user status updates because those flows have different
 * authorization and transaction boundaries. Implementations live in
 * {@code service.impl} and own the concrete repository calls.
 */
package com.oj.problem.service;
