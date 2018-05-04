// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/wasm/regression-727222.js");
	}
}
load('test/mjsunit/wasm/wasm-constants.js');
load('test/mjsunit/wasm/wasm-module-builder.js');

var builder = new WasmModuleBuilder();
builder.addMemory(0, 0, false);
builder.addFunction('f', kSig_i_v)
    .addBody([kExprMemorySize, kMemoryZero])
    .exportFunc();
var instance = builder.instantiate();
instance.exports.f();
