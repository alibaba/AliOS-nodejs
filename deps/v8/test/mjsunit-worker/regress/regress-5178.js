// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-5178.js");
	}
}
assertThrows(() => {
  try { throw {} } catch({a=b, b}) { a+b }
}, ReferenceError);

try { throw {a: 42} } catch({a, b=a}) { assertEquals(42, b) };
