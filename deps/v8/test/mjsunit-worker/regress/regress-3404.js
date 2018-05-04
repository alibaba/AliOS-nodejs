// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-3404.js");
	}
}
function testError(error) {
  // Reconfigure e.stack to be non-configurable
  var desc1 = Object.getOwnPropertyDescriptor(error, "stack");
  Object.defineProperty(error, "stack",
                        {get: desc1.get, set: desc1.set, configurable: false});

  var desc2 = Object.getOwnPropertyDescriptor(error, "stack");
  assertFalse(desc2.configurable);
  assertEquals(desc1.get, desc2.get);
  assertEquals(desc2.get, desc2.get);
}

function stackOverflow() {
  function f() { f(); }
  try { f() } catch (e) { return e; }
}

function referenceError() {
  try { g() } catch (e) { return e; }
}

testError(referenceError());
testError(stackOverflow());
