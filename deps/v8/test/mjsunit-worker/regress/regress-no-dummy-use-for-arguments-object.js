// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-no-dummy-use-for-arguments-object.js");
	}
}
function g() {
  arguments.length;
}

var global = "";

function f() {
  global.dummy = this;
  g({});
}

f();
f();
%OptimizeFunctionOnNextCall(f);
f();
