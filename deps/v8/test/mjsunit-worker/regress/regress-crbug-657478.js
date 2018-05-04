// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-657478.js");
	}
}
function foo(o) { return %_ToLength(o.length); }

foo(new Array(4));
foo(new Array(Math.pow(2, 32) - 1));
foo({length: 10});
%OptimizeFunctionOnNextCall(foo);
foo({length: 10});
