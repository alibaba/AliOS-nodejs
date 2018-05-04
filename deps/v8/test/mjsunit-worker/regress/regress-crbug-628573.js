// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-628573.js");
	}
}
var z = {valueOf: function() { return 3; }};

(function() {
  try {
    var tmp = { x: 12 };
    with (tmp) {
      z++;
    }
    throw new Error("boom");
  } catch(e) {}
})();
