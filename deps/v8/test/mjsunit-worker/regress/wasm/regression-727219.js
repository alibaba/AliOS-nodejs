// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --expose-gc --validate-asm

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/wasm/regression-727219.js");
	}
}
function asm() {
  "use asm";
  function f(a) {
    a = a | 0;
    tab[a & 0]() | 0;
  }
  function unused() {
    return 0;
  }
  var tab = [ unused ];
  return f;
}

asm();
gc();
asm();
