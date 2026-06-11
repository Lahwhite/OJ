/**
 * HTTP interface layer for the problem module.
 *
 * Controllers in this package keep request handling thin: they receive query
 * parameters or JSON bodies, delegate business decisions to services, and wrap
 * the result with {@code ApiResponse}. Admin-only endpoints validate JWTs here
 * so service methods can assume that caller identity has already been parsed.
 * The judge callback endpoint is protected by a shared service token because it
 * is called by another module instead of an interactive browser user.
 */
package com.oj.problem.controller;
