// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --expose-gc

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-590074.js");
	}
}
var __v_5 = {};

function __f_10() {
  var __v_2 = [0, 0, 0];
  __v_2[0] = 0;
  gc();
  return __v_2;
}

function __f_2(array) {
  array[1] = undefined;
}

function __f_9() {
  var __v_4 = __f_10();
  __f_2(__f_10());
  __v_5 = __f_10();
  __v_4 = __f_10();
  __f_2(__v_5);
}
__f_9();
%OptimizeFunctionOnNextCall(__f_9);
__f_9();
