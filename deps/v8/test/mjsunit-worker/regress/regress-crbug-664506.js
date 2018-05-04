// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --expose-gc --predictable --random-seed=-1109634722

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-664506.js");
	}
}
gc();
gc();
assertEquals("[object Object]", Object.prototype.toString.call({}));
gc();
assertEquals("[object Array]", Object.prototype.toString.call([]));
