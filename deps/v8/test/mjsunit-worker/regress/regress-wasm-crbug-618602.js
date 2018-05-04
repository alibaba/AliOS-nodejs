// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --validate-asm --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-wasm-crbug-618602.js");
	}
}
function __f_1() {
  'use asm';
  function __f_3() {
    var __v_11 = 1, __v_10 = 0, __v_12 = 0;
    __v_12 = (__v_10 | 12) % 4294967295 | -1073741824;
  }
  return { __f_3: __f_3 };
}
assertFalse(%IsAsmWasmCode(__f_1));
