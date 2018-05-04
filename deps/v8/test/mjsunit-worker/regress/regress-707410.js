// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-707410.js");
	}
}
var a = new Uint8Array(1024*1024);
%ArrayBufferNeuter(a.buffer);
var b = new Uint8Array(a);
assertEquals(0, b.length);
