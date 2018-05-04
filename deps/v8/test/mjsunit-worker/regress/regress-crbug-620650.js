// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-620650.js");
	}
}
(function() {
  function f(src, dst, i) {
    dst[i] = src[i];
  }
  var buf = new ArrayBuffer(16);
  var view_int32 = new Int32Array(buf);
  view_int32[1] = 0xFFF7FFFF;
  var view_f64 = new Float64Array(buf);
  var arr = [,0.1];
  f(view_f64, arr, -1);
  f(view_f64, arr, 0);
})();
