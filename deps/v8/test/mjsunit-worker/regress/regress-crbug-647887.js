// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-647887.js");
	}
}
function f(obj) {
  var key;
  for (key in obj) { }
  return key === undefined;
}

%OptimizeFunctionOnNextCall(f);
assertFalse(f({ foo:0 }));
