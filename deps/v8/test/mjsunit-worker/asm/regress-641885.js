// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/asm/regress-641885.js");
	}
}
var __f_2 = (function __f_4() {
  "use asm";
  function __f_2(i) {
    i = i|0;
    i = i << -2147483648 >> -1073741824;
    return i|0;
  }
  return { __f_2: __f_2 };
})().__f_2;
