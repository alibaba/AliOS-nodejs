// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --use-osr

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/osr-function-id2.js");
	}
}
function id(f) { return f; }

var x = (function foo() {
  var sum = 0;
  var r = id(foo);
  for (var i = 0; i < 100000; i++) {
    sum += i;
  }
  return foo == r;
})();

assertEquals(true, x);

var x = (function bar() {
  var sum = 0;
  for (var i = 0; i < 90000; i++) {
    sum += i;
  }
  return bar;
})();

assertEquals("function", typeof x);
