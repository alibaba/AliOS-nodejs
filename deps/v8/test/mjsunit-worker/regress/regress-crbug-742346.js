// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --expose-gc --throws

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-742346.js");
	}
}
TypeError.prototype.__defineGetter__("name", function() {
  this[1] = {};
  gc();
  new Uint16Array().reduceRight();
});
var v = WebAssembly.compile();
new TypeError().toString();
