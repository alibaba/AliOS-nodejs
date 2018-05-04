// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-443744.js");
	}
}
var m = (function(stdlib, foreign, heap) {
  "use asm";
  var MEM = new stdlib.Uint8Array(heap);
  function f(x) {
    x = x | 0;
    MEM[x] = 0;
  }
  return {f: f};
})(this, {}, new ArrayBuffer(1));
m.f(-926416896 * 32 * 1024);
