// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-6607-2.js");
	}
}
function get(a, i) {
  return a[i];
}

get([1,,3], 0);
get([1,,3], 2);
%OptimizeFunctionOnNextCall(get);
get([1,,3], 0);
assertOptimized(get);

// This unrelated change to the Object.prototype should be fine.
Object.prototype.unrelated = 1;
assertOptimized(get);
