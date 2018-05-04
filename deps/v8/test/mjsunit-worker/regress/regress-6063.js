// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-6063.js");
	}
}
var U16 = new Uint16Array(2);
U16[0] = 0xffff;

function foo(a, i) {
  return U16[0] === 0xffff;
}

assertTrue(foo());
assertTrue(foo());
%OptimizeFunctionOnNextCall(foo);
assertTrue(foo());
