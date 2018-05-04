// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --turbo-escape --noanalyze-environment-liveness

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-613494.js");
	}
}
function f() {
  var bound = 0;
  function g() { return bound }
}
f();
f();
%OptimizeFunctionOnNextCall(f);
f();
