// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Check that stack overflow inside asm-wasm translation propagates correctly.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/wasm/regression-717056.js");
	}
}
function asm() {
  'use asm';
  return {};
}

function rec() {
  asm();
  rec();
}
assertThrows(() => rec(), RangeError);
