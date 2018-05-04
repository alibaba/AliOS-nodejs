// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --lazy-inner-functions

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//lazy-inner-functions.js");
	}
}
(function TestLazyInnerFunctionCallsEval() {
  var i = (function eager_outer() {
    var outer_var = 41; // Should be context-allocated
    function lazy_inner() {
      return eval("outer_var");
    }
    return lazy_inner;
  })();
  assertEquals(41, i());
})();

(function TestLazyInnerFunctionDestructuring() {
  var i = (function eager_outer() {
    var outer_var = 41; // Should be context-allocated
    function lazy_inner() {
      // This introduces b and refers to outer_var.
      var {outer_var : b} = {outer_var};
      return b;
    }
    return lazy_inner;
  })();
  assertEquals(41, i());
})();
