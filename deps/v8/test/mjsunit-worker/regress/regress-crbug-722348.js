// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-722348.js");
	}
}
function Module(global, env) {
  "use asm";
  var unused_fun = env.fun;
  function f() {}
  return { f:f }
}
assertThrows(() => Module(), TypeError);
assertFalse(%IsAsmWasmCode(Module));
