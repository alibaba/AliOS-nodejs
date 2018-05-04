// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --use-osr --noalways-opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-480807.js");
	}
}
function foo() {
  var c = 0;
  for (var e = 0; e < 1; ++e) {
    for (var a = 1; a > 0; a--) {
      c += 1;
    }
    for (var b = 1; b > 0; b--) {
      %OptimizeOsr();
    }
  }
  return c;
}
try {
  foo();
} catch (e) {
}
