// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --turbo-filter=* --always-opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-467531.js");
	}
}
assertThrows(function() {
  "use strict";
  try {
    x = ref_error;
    let x = 0;
  } catch (e) {
    throw e;
  }
}, ReferenceError);

assertThrows(function() {
  "use strict";
  try {
    x = ref_error;
    let x = 0;
  } finally {
    // re-throw
  }
}, ReferenceError);
