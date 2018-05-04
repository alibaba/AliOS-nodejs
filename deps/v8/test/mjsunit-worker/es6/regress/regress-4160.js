// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-4160.js");
	}
}
(function(x) {
  (function(x) {
    var boom = (() => eval(x));
    assertEquals(23, boom());
    assertEquals(23, boom());
    %OptimizeFunctionOnNextCall(boom);
    assertEquals(23, boom());
    assertEquals("23", x);
  })("23");
  assertEquals("42", x);
})("42");

(function(x) {
  (function(x) {
    var boom = (() => (eval("var x = 66"), x));
    assertEquals(66, boom());
    assertEquals(66, boom());
    %OptimizeFunctionOnNextCall(boom);
    assertEquals(66, boom());
    assertEquals("23", x);
  })("23");
  assertEquals("42", x);
})("42");
