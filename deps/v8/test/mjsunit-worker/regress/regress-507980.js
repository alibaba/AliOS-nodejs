// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-507980.js");
	}
}
__v_1 = new Float64Array(1);
__v_8 = { valueOf: function() { __v_13.y = "bar"; return 42; }};
__v_13 = __v_1;
__v_13[0] = __v_8;
