"use strict"
// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-608278.js");
	}
}
"use strict";

function h() {
  var stack = (new Error("boom")).stack;
  print(stack);
  %DeoptimizeFunction(f1);
  %DeoptimizeFunction(f2);
  %DeoptimizeFunction(f3);
  %DeoptimizeFunction(g);
  %DeoptimizeFunction(h);
  return 1;
}
%NeverOptimizeFunction(h);

function g(v) {
  return h();
}
%SetForceInlineFlag(g);


function f1() {
  var o = {};
  o.__defineGetter__('p', g);
  o.p;
}

f1();
f1();
%OptimizeFunctionOnNextCall(f1);
f1();


function f2() {
  var o = {};
  o.__defineSetter__('q', g);
  o.q = 1;
}

f2();
f2();
%OptimizeFunctionOnNextCall(f2);
f2();


function A() {
  return h();
}

function f3() {
  new A();
}

f3();
f3();
%OptimizeFunctionOnNextCall(f3);
f3();
