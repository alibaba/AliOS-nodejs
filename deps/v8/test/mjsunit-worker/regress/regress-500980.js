// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-500980.js");
	}
}
var a = "a";
assertThrows(function() { while (true) a += a; }, RangeError);
assertThrows(function() { a in a; }, TypeError);
