// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-714971.js");
	}
}
function Module(stdlib, foreign, heap) {
  "use asm";
  var a = new stdlib.Int16Array(heap);
  function f() {
    return a[23 >> -1];
  }
  return { f:f };
}
var b = new ArrayBuffer(1024);
var m = Module(this, {}, b);
new Int16Array(b)[0] = 42;
assertEquals(42, m.f());
assertFalse(%IsAsmWasmCode(Module));
