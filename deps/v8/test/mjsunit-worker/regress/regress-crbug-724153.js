// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --no-turbo-verify

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-724153.js");
	}
}
(function TestParameterLimit() {
  var src = '(function f(a,';
  for (var i = 0; i < 65535 - 2; i++) {
    src += 'b' + i + ',';
  }
  src += 'c) { return a + c })';
  var f = eval(src);
  assertEquals(NaN, f(1));
  assertEquals(NaN, f(2));
  %OptimizeFunctionOnNextCall(f);
  assertEquals(NaN, f(3));
})();
