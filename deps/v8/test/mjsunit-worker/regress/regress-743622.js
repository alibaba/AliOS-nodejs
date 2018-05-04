// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --validate-asm

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-743622.js");
	}
}
function Module(stdlib, foreign, heap) {
  "use asm";
  var a = stdlib.Math.PI;
  function f() { return a }
  return { f:f };
}
Module.length
