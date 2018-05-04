// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-arg-materialize-store.js");
	}
}
function f() {
  return f.arguments;
}

function g(deopt) {
  var o = { x : 2 };
  f();
  o.x = 1;
  deopt + 0;
  return o.x;
}

g(0);
g(0);
%OptimizeFunctionOnNextCall(g);
assertEquals(1, g({}));
