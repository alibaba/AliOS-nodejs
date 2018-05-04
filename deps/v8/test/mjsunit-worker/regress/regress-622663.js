if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-622663.js");
	}
}
+// Copyright 2016 the V8 project authors. All rights reserved.
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+// Flags: --no-lazy

(function() {
  try { (y = [...[]]) => {} } catch(_) {}  // will core dump, if not fixed
})();

(function() {
  try { ((y = [...[]]) => {})(); } catch(_) {}  // will core dump, if not fixed,
                                                // even without --no-lazy
})();
