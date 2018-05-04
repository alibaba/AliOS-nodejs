// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-696622.js");
	}
}
class C {}
class D extends C { constructor() { super(...unresolved, 75) } }
D.__proto__ = null;

assertThrows(() => new D(), TypeError);
assertThrows(() => new D(), TypeError);
%OptimizeFunctionOnNextCall(D);
assertThrows(() => new D(), TypeError);
