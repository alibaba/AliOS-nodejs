// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-644215.js");
	}
}
var arr = [...[],,];
assertTrue(%HasHoleyElements(arr));
assertEquals(1, arr.length);
assertFalse(arr.hasOwnProperty(0));
assertEquals(undefined, arr[0]);
// Should not crash.
assertThrows(() => arr[0][0], TypeError);
