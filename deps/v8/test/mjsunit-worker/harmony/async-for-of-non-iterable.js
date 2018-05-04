// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --harmony-async-iteration
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/harmony/async-for-of-non-iterable.js");
	}
}
var done = false;

async function f() {
    try {
        for await ([a] of {}) {
            UNREACHABLE();
        }
        UNREACHABLE();
    } catch (e) {
        assertEquals(e.message, "{} is not async iterable");
        done = true;
    }
}

f();
assertTrue(done);
