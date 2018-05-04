// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-668760.js");
	}
}
function f() {
  try {
    o;
  } catch (e) {
    return e;
  }
  return 0;
}

function deopt() {
  %DeoptimizeFunction(f);
  throw 42;
}

%NeverOptimizeFunction(deopt);

this.__defineGetter__("o", deopt );

f();
f();
%OptimizeFunctionOnNextCall(f);
assertEquals(42, f());
