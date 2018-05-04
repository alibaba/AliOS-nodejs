// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-433332.js");
	}
}
function f(foo) {
  var g;
  true ? (g = 0.1) : g |=  null;
  if (null != g) {}
};

f(1.4);
f(1.4);
%OptimizeFunctionOnNextCall(f);
f(1.4);
