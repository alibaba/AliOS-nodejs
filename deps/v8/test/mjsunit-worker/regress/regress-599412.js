// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-599412.js");
	}
}
function h(a) {
  if (!a) return false;
  print();
}

function g(a) { return a.length; }
g('0');
g('1');

function f() {
  h(g([]));
}

f();
%OptimizeFunctionOnNextCall(f);
f();
