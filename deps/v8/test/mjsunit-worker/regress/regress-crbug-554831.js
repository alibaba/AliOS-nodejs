// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-554831.js");
	}
}
(function() {
  var key = "s";
  function f(object) { return object[key]; };
  f("");
  f("");
  %OptimizeFunctionOnNextCall(f);
  f("");
  assertOptimized(f);
})();
