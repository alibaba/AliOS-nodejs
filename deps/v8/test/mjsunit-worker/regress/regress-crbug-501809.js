// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --harmony-sharedarraybuffer
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-501809.js");
	}
}
var sab = new SharedArrayBuffer(8);
var ta = new Int32Array(sab);
ta.__defineSetter__('length', function() {;});
assertThrows(function() {
  Atomics.compareExchange(ta, 4294967295, 0, 0);
}, RangeError);
