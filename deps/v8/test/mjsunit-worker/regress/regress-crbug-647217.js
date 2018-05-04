// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --stack-size=100 --no-stress-fullcodegen

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-647217.js");
	}
}
var source = "return 1" + new Array(2048).join(' + a') + "";
eval("function g(a) {" + source + "}");
%SetForceInlineFlag(g);

function f(a) { return g(a) }
%OptimizeFunctionOnNextCall(f);
try { f(0) } catch(e) {}
