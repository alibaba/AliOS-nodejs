// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/asm/float64array-store-div.js");
	}
}
function Module(stdlib, foreign, heap) {
  "use asm";
  var MEM64 = new stdlib.Float64Array(heap);
  function foo(i) {
    i = i | 0;
    MEM64[0] = +(i >>> 0) / 2.;
    return +MEM64[0];
  }
  return { foo: foo };
}

var foo = Module(this, {}, new ArrayBuffer(64 * 1024)).foo;
assertEquals(0.5, foo(1));
