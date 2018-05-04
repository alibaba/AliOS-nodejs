"use strict"
// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// NO HARNESS

if (!isworker()) {
  var ThreadWorkerCount = 1;
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-498811.js");
	}
}
"use strict";
let l = 0;
eval("eval('this')");
