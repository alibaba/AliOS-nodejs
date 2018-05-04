// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// https://code.google.com/p/chromium/issues/detail?id=578775

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-578775.js");
	}
}
var __v_9 = {};
for (var __v_0 = 0; __v_0 < 1000; __v_0++) {
}
__v_2 = { __v_2: 1 };
__v_12 = new Proxy({}, {});
function f() {
  var __v_10 = new Proxy({}, __v_2);
  __v_9.__proto__ = __v_10;
  __v_2.getPrototypeOf = function () { return __v_9 };
  Object.prototype.isPrototypeOf.call(__v_0, __v_10);
};
assertThrows(f, RangeError);
