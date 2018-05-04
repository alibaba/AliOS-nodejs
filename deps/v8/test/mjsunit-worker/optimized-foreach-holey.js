// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --expose-gc --turbo-inline-array-builtins

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//optimized-foreach-holey.js");
	}
}
(function() {
  var result = 0;
  var f = function() {
    var b = [,,];
    b[0] = 0;
    b[2] = 2;
    var sum = function(v,i,o) {
      result += i;
    };
    b.forEach(sum);
  }
  f();
  f();
  %OptimizeFunctionOnNextCall(f);
  f();
  f();
  f();
  assertEquals(10, result);
})();
