// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-683617.js");
	}
}
global = 'n';
function f(deopt) {
  let it = global[Symbol.iterator]();
  if (deopt) {
    return it.next().value;
  }
}
f();
f();
%OptimizeFunctionOnNextCall(f);
assertEquals('n', f(true));
