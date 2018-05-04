// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-549162.js");
	}
}
var s = Symbol("foo");
var __v_13 = {}
Object.defineProperty( __v_13, s, {value: {}, enumerable: true});
for (var __v_14 in __v_13) {}
__v_13 = {}
Object.defineProperty( __v_13, s, {value: {}, enumerable: true});
var __v_14 = Object.create(Object.prototype, __v_13)
