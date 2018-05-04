// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/wasm/wasm-default.js");
	}
}
function GetWebAssembly() {
  return WebAssembly;
}

let WASM_ON_BY_DEFAULT = true;

if (WASM_ON_BY_DEFAULT) {
  try {
    assertFalse(undefined === GetWebAssembly());
  } catch (e) {
    assertTrue(false);
  }
} else {
  assertThrows(GetWebAssembly);
}
