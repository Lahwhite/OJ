/**
 * Business logic implementations for the problem module.
 *
 * Classes here coordinate repositories, transactions, judge-engine integration,
 * and DTO mapping. The implementation layer is where aggregate synchronization
 * happens: test cases are rebuilt under the problem aggregate, tags update their
 * reference counts, and judge results are folded back into problem/user status
 * records. Controllers should stay thin and avoid duplicating these rules.
 */
package com.oj.problem.service.impl;
