// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --always-opt --expose-gc

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-476488.js");
	}
}
function __f_0(message, a) {
  eval(), message;
  (function blue() {
    'use strict';
    eval(), eval(), message;
    gc();
  })();
}
__f_0();
