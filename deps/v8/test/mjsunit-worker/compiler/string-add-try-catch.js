// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/string-add-try-catch.js");
	}
}
var a = "a".repeat(268435440);

(function() {
  function foo(a, b) {
    try {
      return a + "0123456789012";
    } catch (e) {
      return e;
    }
  }

  foo("a");
  foo("a");
  %OptimizeFunctionOnNextCall(foo);
  foo("a");
  assertInstanceof(foo(a), RangeError);
})();

(function() {
  function foo(a, b) {
    try {
      return "0123456789012" + a;
    } catch (e) {
      return e;
    }
  }

  foo("a");
  foo("a");
  %OptimizeFunctionOnNextCall(foo);
  foo("a");
  assertInstanceof(foo(a), RangeError);
})();
