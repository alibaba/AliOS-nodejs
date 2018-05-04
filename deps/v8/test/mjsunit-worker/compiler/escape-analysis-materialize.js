// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --turbo-escape

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/escape-analysis-materialize.js");
	}
}
(function TestMaterializeArray() {
  function f() {
    var a = [1,2,3];
    %_DeoptimizeNow();
    return a.length;
  }
  assertEquals(3, f());
  assertEquals(3, f());
  %OptimizeFunctionOnNextCall(f);
  assertEquals(3, f());
})();

(function TestMaterializeFunction() {
  function g() {
    function fun(a, b) {}
    %_DeoptimizeNow();
    return fun.length;
  }
  assertEquals(2, g());
  assertEquals(2, g());
  %OptimizeFunctionOnNextCall(g);
  assertEquals(2, g());
})();
