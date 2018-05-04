// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --harmony-do-expressions

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/harmony/regress/regress-4904.js");
	}
}
(function testCatchScopeInDoExpression() {
  var f = (s = 17, y = do { try { throw 25; } catch(e) { s += e; }; }) => s;
  var result = f();
  assertEquals(result, 42);
})();

(function testCatchScopeInDoExpression() {
  var f = (s = 17, y = do { let t; try { throw 25; } catch(e) { s += e; }; }) => s;
  var result = f();
  assertEquals(result, 42);
})();

(function testCatchScopeInDoExpression() {
  let t1;
  var f = (s = 17, y = do { let t2; try { throw 25; } catch(e) { s += e; }; }) => s;
  var result = f();
  assertEquals(result, 42);
})();
