// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-4206.js");
	}
}
function Module(stdlib) {
  "use asm";
  function TernaryMin(a, b) {
    a=+(a);
    b=+(b);
    return (+((a < b) ? a : b));
  }
  function TernaryMax(a, b) {
    a=+(a);
    b=+(b);
    return (+((b < a) ? a : b));
  }
  return { TernaryMin: TernaryMin,
           TernaryMax: TernaryMax };
}
var min = Module(this).TernaryMin;
var max = Module(this).TernaryMax;

assertEquals(0.0, min(-0.0, 0.0));
assertEquals(0.0, min(NaN, 0.0));
assertEquals(-0.0, min(NaN, -0.0));
assertEquals(-0.0, max(0.0, -0.0));
assertEquals(0.0, max(NaN, 0.0));
assertEquals(-0.0, max(NaN, -0.0));
