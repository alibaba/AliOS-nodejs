// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-592340.js");
	}
}
class MyArray extends Array { }
Object.prototype[Symbol.species] = MyArray;
delete Array[Symbol.species];
__v_1 = Math.pow(2, 31);
__v_2 = [];
__v_2[__v_1] = 31;
__v_4 = [];
__v_4[__v_1 - 2] = 33;
assertThrows(() => __v_2.concat(__v_4), RangeError);
