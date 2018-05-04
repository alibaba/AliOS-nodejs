// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-393988.js");
	}
}
var o = {};
Error.captureStackTrace(o);
Object.defineProperty(o, "stack", { value: 1 });
assertEquals(1, o.stack);
