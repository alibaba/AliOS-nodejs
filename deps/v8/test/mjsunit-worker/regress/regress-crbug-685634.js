"use strict"
// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-685634.js");
	}
}
"use strict";

function foo(f) { return f.apply(this, arguments); }
function bar() {}

foo(bar);
%OptimizeFunctionOnNextCall(foo);
foo(bar);
