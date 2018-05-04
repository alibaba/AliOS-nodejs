// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-411237.js");
	}
}
try {
  %OptimizeFunctionOnNextCall(print);
} catch(e) { }

try {
  function* f() {
  }
  %OptimizeFunctionOnNextCall(f);
} catch(e) { }
