// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-6142.js");
	}
}
try {
  eval('a: { continue a; }');
  assertUnreachable();
} catch(e) {
  assertTrue(e instanceof SyntaxError);
  assertEquals('Illegal continue statement: \'a\' does not denote an iteration statement', e.message);
}

try {
  eval('continue;');
  assertUnreachable();
} catch(e) {
  assertTrue(e instanceof SyntaxError);
  assertEquals('Illegal continue statement: no surrounding iteration statement', e.message);
}

try {
  eval('a: { continue b;}');
  assertUnreachable();
} catch(e) {
  assertTrue(e instanceof SyntaxError);
  assertEquals("Undefined label 'b'", e.message);
}
