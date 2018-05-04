// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// See http://code.google.com/p/v8/issues/detail?id=5214


if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-5214.js");
	}
}
assertEquals(Infinity, Math.pow(2, 0x80000000));
assertEquals(Infinity, Math.pow(2, 0xc0000000));
assertEquals(0, Math.pow(2, -0x80000000));
