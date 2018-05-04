// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/wasm/regression-711203.js");
	}
}
load("test/mjsunit/wasm/wasm-constants.js");
load("test/mjsunit/wasm/wasm-module-builder.js");

(function() {
  var builder = new WasmModuleBuilder();
  builder.addMemory(16, 32, false);
  builder.addFunction("test", kSig_i_iii)
      .addBodyWithEnd([
        // body:
        kExprI64Const, 0,
        kExprI64Const, 0x1,
        kExprI64Clz,
        kExprI64Sub,
        kExprI64Const, 0x10,
        kExprI64Const, 0x1b,
        kExprI64Shl,
        kExprI64Sub,
        kExprI64Popcnt,
        kExprI32ConvertI64,
        kExprEnd,   // @207
      ])
            .exportFunc();
  var module = builder.instantiate();
  const result = module.exports.test(1, 2, 3);
  assertEquals(58, result);
})();
