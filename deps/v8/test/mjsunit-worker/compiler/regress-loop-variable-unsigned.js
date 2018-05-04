// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --turbo-loop-variable

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-loop-variable-unsigned.js");
	}
}
(function() {
  function f() {
    for (var i = 0; i < 4294967295; i += 2) {
      if (i === 10) break;
    }
  }
  f();
})();

(function() {
  function f() {
    for (var i = 0; i < 4294967293; i += 2) {
      if (i === 10) break;
    }
  }
  f();
})();
