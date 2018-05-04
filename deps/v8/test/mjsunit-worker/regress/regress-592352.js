// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --validate-asm

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-592352.js");
	}
}
function __f_76() {
  "use asm";
  function __f_72() {
    %OptimizeFunctionOnNextCall();
  }
  return {__f_72:__f_72};
}

try {
  assertTrue(%IsAsmWasmCode(__f_76));
  assertTrue(false);
} catch (e) {
  print("Caught: " + e);
}
