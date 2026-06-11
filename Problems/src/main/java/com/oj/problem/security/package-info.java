/**
 * Authentication helpers local to the problem module.
 *
 * The module only needs a small slice of user identity: user id and role parsed
 * from the shared JWT. Keeping that parsing here lets controllers protect admin
 * operations without depending directly on the Users module runtime. If token
 * claims change, this package is the compatibility layer that should be updated
 * before service logic is touched.
 */
package com.oj.problem.security;
