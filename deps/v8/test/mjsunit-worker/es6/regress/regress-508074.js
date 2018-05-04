// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-508074.js");
	}
}
var f = (a, b, ...c) => {
  print(a);
  print(b);
  print(c);
  assertEquals(6, a);
  assertEquals(5, b);
  assertEquals([4, 3, 2, 1], c);
};

function g() {
  f(6, 5, 4, 3, 2, 1);
};

g();
g();
g();

%OptimizeFunctionOnNextCall(g);
g();
