// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --expose-gc

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-741078.js");
	}
}
function* gen() {}

(function warmup() {
  for (var i = 0; i < 100; ++i) {
    var g = gen();
    g.p = 42;
  }
})();

gc();   // Ensure no instance alive.
gen();  // Still has unused fields.
%OptimizeFunctionOnNextCall(gen);
gen();  // Was shrunk, boom!
