var vows = require('vows'), assert = require('assert');
var controlflow = require('async');

// Test websocket client connection, no identity

// Test websocket client connection, 5 times
/**
 * 1. let websocket client connect, transfer core identity
 * 2. client: transfer identity
 * 3. Core: check if property listeners are correct
 * 4. continue with 1
 */

// Test websocket client disconnect, all
/**
 * 1. let websocket client connect, transfer core identity
 * 2. client: transfer identity
 * 3. Core: check if property listeners are correct
 * 4. continue with 1
 */