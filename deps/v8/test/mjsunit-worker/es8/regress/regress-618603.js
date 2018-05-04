// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es8/regress/regress-618603.js");
	}
}
try {
} catch(e) {; }
function __f_7(expected, run) {
  var __v_10 = run();
};
__f_7("[1,2,3]", () => (function() {
      return (async () => {[...await arguments] })();
      })());
