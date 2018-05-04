// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es7/regress/regress-634269.js");
	}
}
__v_1 = new Uint8Array();
Object.defineProperty(__v_1.__proto__, 'length', {value: 42});
Array.prototype.includes.call(new Uint8Array(), 2);
