// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-468162.js");
	}
}
var asm = (function() {
  "use asm";
  var max = Math.max;
  return function f() { return max(0, -17); };
})();

assertEquals(0, asm());
