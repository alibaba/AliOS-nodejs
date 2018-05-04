// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-385054.js");
	}
}
function f(x) {
  var a = [1, 2];
  a[x];
  return a[0 - x];
}

f(0);
f(0);
%OptimizeFunctionOnNextCall(f);
assertEquals(undefined, f(1));
