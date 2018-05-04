// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-523308.js");
	}
}
var error;
try { reference_error(); } catch (e) { error = e; }
toString = error.toString;
error.__proto__ = [];
assertEquals("Error: reference_error is not defined", toString.call(error));
