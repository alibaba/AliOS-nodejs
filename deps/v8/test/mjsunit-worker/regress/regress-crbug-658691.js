// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --no-stress-fullcodegen

// The {f} function is compiled using TurboFan.
// 1) The call to {Reflect.set} has no arguments adaptation.
// 2) The call to {Reflect.set} is in tail position.
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-658691.js");
	}
}
function f(a, b, c) {
  "use strict";
  return Reflect.set({});
}

// The {g} function is compiled using Ignition.
// 1) The call to {f} requires arguments adaptation.
// 2) The call to {f} is not in tail position.
function g() {
  return f() + "-no-tail";
}

assertEquals("true-no-tail", g());
%OptimizeFunctionOnNextCall(f);
assertEquals("true-no-tail", g());
