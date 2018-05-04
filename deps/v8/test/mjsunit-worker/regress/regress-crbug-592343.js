// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-592343.js");
	}
}
var r = /[^\u{1}-\u{1000}\u{1002}-\u{2000}]/u;
assertTrue(r.test("\u{0}"));
assertFalse(r.test("\u{1}"));
assertFalse(r.test("\u{1000}"));
assertTrue(r.test("\u{1001}"));
assertFalse(r.test("\u{1002}"));
assertFalse(r.test("\u{2000}"));
assertTrue(r.test("\u{2001}"));
