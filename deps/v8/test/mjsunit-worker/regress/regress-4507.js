// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-4507.js");
	}
}
function broken(value) {
  return Math.floor(value/65536);
}
function toUnsigned(i) {
  return i >>> 0;
}
function outer(i) {
  return broken(toUnsigned(i));
}
for (var i = 0; i < 5; i++) outer(0);
broken(0x80000000);  // Spice things up with a sprinkling of type feedback.
%OptimizeFunctionOnNextCall(outer);
assertEquals(32768, outer(0x80000000));
