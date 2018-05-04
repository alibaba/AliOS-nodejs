// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-arguments-liveness-analysis.js");
	}
}
function r(v) { return v.f }
function h() { }
function y(v) {
  var x = arguments;
  h.apply(r(v), x);
};

y({f:3});
y({f:3});
y({f:3});

%OptimizeFunctionOnNextCall(y);

y({ f : 3, u : 4 });
