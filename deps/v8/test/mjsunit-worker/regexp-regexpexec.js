// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Test that the fallback path in RegExpExec executes the default exec
// implementation.
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//regexp-regexpexec.js");
	}
}
delete RegExp.prototype.exec;
assertEquals("b", "aba".replace(/a/g, ""));
