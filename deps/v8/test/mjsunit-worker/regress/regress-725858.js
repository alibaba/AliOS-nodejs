// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags:  --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-725858.js");
	}
}
function f() {}
var src = 'f(' + '0,'.repeat(0x201f) + ')';
var boom = new Function(src);
%OptimizeFunctionOnNextCall(boom);
boom();
