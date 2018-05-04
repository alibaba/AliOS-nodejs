// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --invoke-weak-callbacks --omit-quit --no-test

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-493284.js");
	}
}
if (this.Intl) {
  var coll = new Intl.Collator();
  assertEquals(-1, coll.compare('a', 'c'));
}
