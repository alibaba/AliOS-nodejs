// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-523307.js");
	}
}
function f(x) {
  var c = x * x << 366;
  var a = c + c;
  return a;
}

f(1);
f(1);
%OptimizeFunctionOnNextCall(f);
f(1);
