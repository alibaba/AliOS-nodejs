// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --harmony-dynamic-import

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/harmony/modules-import-4.js");
	}
}
try {
  import(x);
  let x;
  assertUnreachable();
} catch(e) {
  assertEquals("ReferenceError", e.name);
}
