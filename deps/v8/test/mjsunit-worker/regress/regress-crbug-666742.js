// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --expose-gc

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-666742.js");
	}
}
var p = {x:1};
__proto__ = p;
assertEquals(x, 1);
__proto__ = {x:13};
assertEquals(x, 13);
__proto__ = {x:42};
p = null;
gc();
assertEquals(x, 42);
