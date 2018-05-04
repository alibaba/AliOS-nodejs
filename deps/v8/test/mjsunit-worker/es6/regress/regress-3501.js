"use strict"
// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// See: http://code.google.com/p/v8/issues/detail?id=3501

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-3501.js");
	}
}
"use strict";
let lift = f => (x, k) => k (f (x));
lift(isNaN);
