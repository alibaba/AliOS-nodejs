// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --check-elimination --stress-opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-352058.js");
	}
}
var v0 = this;
var v2 = this;
function f() {
  v2 = [1.2, 2.3];
  v0 = [12, 23];
}

f();
f();
%OptimizeFunctionOnNextCall(f);
f();
