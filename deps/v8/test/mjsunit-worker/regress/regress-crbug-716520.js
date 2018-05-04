// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-716520.js");
	}
}
var __v_0 = {};
var __v_8 = this;
var __v_11 = -1073741825;
__v_1 = this;
try {
} catch(e) {; }
  function __f_4() {}
  __f_4.prototype = __v_0;
  function __f_9() { return new __f_4().v; }
  __f_9(); __f_9();
try {
(function() {
})();
} catch(e) {; }
  Object.assign(__v_0, __v_1, __v_0);
(function() {
})();
