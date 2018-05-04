// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-729573-1.js");
	}
}
(function() {
  function foo() {
    var a = foo.bind(this);
    %DeoptimizeNow();
    if (!a) return a;
    return 0;
  }

  assertEquals(0, foo());
  assertEquals(0, foo());
  %OptimizeFunctionOnNextCall(foo);
  assertEquals(0, foo());
})();

(function() {
  "use strict";

  function foo() {
    var a = foo.bind(this);
    %DeoptimizeNow();
    if (!a) return a;
    return 0;
  }

  assertEquals(0, foo());
  assertEquals(0, foo());
  %OptimizeFunctionOnNextCall(foo);
  assertEquals(0, foo());
})();

(function() {
  function foo() {
    var a = foo.bind(this);
    %DeoptimizeNow();
    if (!a) return a;
    return 0;
  }
  foo.prototype = {custom: "prototype"};

  assertEquals(0, foo());
  assertEquals(0, foo());
  %OptimizeFunctionOnNextCall(foo);
  assertEquals(0, foo());
})();

(function() {
  "use strict";

  function foo() {
    var a = foo.bind(this);
    %DeoptimizeNow();
    if (!a) return a;
    return 0;
  }
  foo.prototype = {custom: "prototype"};

  assertEquals(0, foo());
  assertEquals(0, foo());
  %OptimizeFunctionOnNextCall(foo);
  assertEquals(0, foo());
})();
