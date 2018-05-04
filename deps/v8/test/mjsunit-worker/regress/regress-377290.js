// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --expose-gc

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-377290.js");
	}
}
Object.prototype.__defineGetter__('constructor', function() { throw 42; });
__v_7 = [
  function() { [].push() },
];
for (var __v_6 = 0; __v_6 < 5; ++__v_6) {
  for (var __v_8 in __v_7) {
    print(__v_8, " -> ", __v_7[__v_8]);
    gc();
    try { __v_7[__v_8](); } catch (e) {};
  }
}
