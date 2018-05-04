// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-650933.js");
	}
}
var a = [0, 1, 2, 3, 4, 5, 6, 7, 8];
var o = {length: 1e40};
try { new Uint8Array(o); } catch (e) { }
new Float64Array(a);
