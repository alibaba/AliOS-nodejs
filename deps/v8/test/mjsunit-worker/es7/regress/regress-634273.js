// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --enable-slow-asserts

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es7/regress/regress-634273.js");
	}
}
array = new Array(undefined, undefined, undefined);
Object.defineProperty(array, 0, {
  get: function() {
    array.push(undefined, undefined);
  }
});
array[0x80000] = 1;
result = array.includes(new WeakMap());
