// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-729671.js");
	}
}
var o = { 0: 11, 1: 9};
assertThrows(() => JSON.parse('[0,0]', function() { this[1] = o; }), RangeError);
