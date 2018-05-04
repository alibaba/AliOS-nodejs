// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --validate-asm --no-stress-fullcodegen --lazy-inner-functions

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/asm/regress-674089.js");
	}
}
function outer() {
  "use asm";
  function inner() {
    switch (1) {
      case 0:
        break foo;
    }
  }
}
outer();
