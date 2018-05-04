// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --expose-wasm

// Old API should be gone.
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/wasm/wasm-object-api.js");
	}
}
assertEquals("undefined", typeof Wasm);

// New API should rule.
assertEquals('object', typeof WebAssembly);
assertEquals('function', typeof WebAssembly.Module);
assertEquals('function', typeof WebAssembly.Instance);
assertEquals('function', typeof WebAssembly.compile);
assertEquals('function', typeof WebAssembly.validate);
