// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax


if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/computed-property-names-deopt.js");
	}
}
(function TestProtoDeopt() {
  var proto = {};

  function deoptMe() {
    %DeoptimizeFunction(f);
    return proto;
  }

  function checkObject(name, value, o) {
    assertSame(proto, Object.getPrototypeOf(o));
    assertTrue(o.hasOwnProperty(name));
    assertEquals(value, o[name]);
  }

  function f(name, value) {
    return { [name]: value, __proto__: deoptMe() };
  }

  checkObject("a", 1, f("a", 1));
  checkObject("b", 2, f("b", 2));
  %OptimizeFunctionOnNextCall(f);
  checkObject("c", 3, f("c", 3));
})();
