// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --harmony-do-expressions

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/harmony/regress/regress-crbug-578038.js");
	}
}
(function testWithoutOtherLiteral() {
  var result = ((x = [...[42]]) => x)();
  assertEquals(result, [42]);
})();

(function testWithSomeOtherLiteral() {
  []; // important: an array literal before the arrow function
  var result = ((x = [...[42]]) => x)();  // will core dump, if not fixed.
  assertEquals(result, [42]);
})();
