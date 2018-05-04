// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-736567.js");
	}
}
function f(b, x) {
  var o = b ? { a : 1 } : undefined;
  return o.a + !(x & 1);
}

f(1);

function g() {
  f(0, "s");
}

assertThrows(g);
%OptimizeFunctionOnNextCall(g);
assertThrows(g);
