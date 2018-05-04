// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//number-isnan-opt.js");
	}
}
(function() {
  function foo(x) { return Number.isNaN(x); }

  assertTrue(foo(+undefined));
  assertFalse(foo(undefined));
  %OptimizeFunctionOnNextCall(foo);
  assertTrue(foo(+undefined));
  assertFalse(foo(undefined));
})();

(function() {
  function foo(x) { return Number.isNaN(+x); }

  assertTrue(foo(+undefined));
  assertFalse(foo(0));
  %OptimizeFunctionOnNextCall(foo);
  assertTrue(foo(+undefined));
  assertFalse(foo(0));
})();

(function() {
  function foo(x) { return Number.isNaN(x|0); }

  assertFalse(foo(+undefined));
  assertFalse(foo(0));
  %OptimizeFunctionOnNextCall(foo);
  assertFalse(foo(+undefined));
  assertFalse(foo(0));
})();

(function() {
  function foo(x) { return Number.isNaN("" + x); }

  assertFalse(foo(undefined));
  assertFalse(foo(0));
  %OptimizeFunctionOnNextCall(foo);
  assertFalse(foo(undefined));
  assertFalse(foo(0));
})();

(function() {
  function foo(x) { return Number.isNaN(0/0); }

  assertTrue(foo());
  assertTrue(foo());
  %OptimizeFunctionOnNextCall(foo);
  assertTrue(foo());
  assertTrue(foo());
})();
