// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --always-opt --function-context-specialization --verify-heap

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-681171-2.js");
	}
}
function* gen() {
  for (var i = 0; i < 3; ++i) {
    yield i
  }
}
gen().next();
