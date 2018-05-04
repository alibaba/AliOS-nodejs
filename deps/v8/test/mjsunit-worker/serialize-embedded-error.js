// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// --serialize-toplevel --cache=code

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//serialize-embedded-error.js");
	}
}
var caught = false;
try {
  parseInt() = 0;
} catch(e) {
  caught = true;
}
assertTrue(caught);
