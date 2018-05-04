// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-5929-1.js");
	}
}
var buf = new ArrayBuffer(0x10000);
var arr = new Uint8Array(buf).fill(55);
var tmp = {};
tmp[Symbol.toPrimitive] = function () {
  %ArrayBufferNeuter(arr.buffer);
  return 50;
}
arr.copyWithin(tmp);
