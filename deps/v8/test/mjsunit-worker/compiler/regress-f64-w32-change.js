// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-f64-w32-change.js");
	}
}
var f = (function () {
  "use asm";
  var f64use = 0;
  function f(x, b) {
    x = x|0;
    b = b >>> 0;
    var f64 = x ? -1 : b;
    f64use = f64 + 0.5;
    var w32 = x ? 1 : f64;
    return (w32 + 1)|0;
  }

  return f;
})();

%OptimizeFunctionOnNextCall(f);
assertEquals(0, f(0, -1));
