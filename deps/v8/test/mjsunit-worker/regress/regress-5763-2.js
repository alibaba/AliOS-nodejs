// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-5763-2.js");
	}
}
try {
  var TA = Object.getPrototypeOf(Int8Array);
  var obj = Reflect.construct(TA, [], Int8Array);
  new Int8Array(4).set(obj);
} catch (e) {}
