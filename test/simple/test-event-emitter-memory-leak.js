// Copyright & License details are available under JXCORE_LICENSE file


// Flags: --expose-gc

// Add and remove a lot of different events to an EventEmitter, then check
// that we didn't leak the event names.
var common = require('../common');
var assert = require('assert');
var events = require('events');

assert.equal(typeof gc, 'function', 'Run this test with --expose-gc');
gc();

var before = process.memoryUsage().heapUsed;
var e = new events.EventEmitter();

for (var i = 0; i < 2.5e5; ++i) {
  var name = 'a-pretty-long-event-name-' + i;
  e.on(name, assert.fail);
  e.removeListener(name, assert.fail);
}
gc();

var after = process.memoryUsage().heapUsed;
assert(after - before < 1024*1024, 'EventEmitter leaks event names.');