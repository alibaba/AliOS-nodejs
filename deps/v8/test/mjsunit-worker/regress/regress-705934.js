// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --harmony-regexp-named-captures --stack-size=100

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-705934.js");
	}
}
function call_replace_close_to_stack_overflow() {
  try {
    call_replace_close_to_stack_overflow();
  } catch(e) {
    "b".replace(/(b)/g);
  }
}

call_replace_close_to_stack_overflow();
