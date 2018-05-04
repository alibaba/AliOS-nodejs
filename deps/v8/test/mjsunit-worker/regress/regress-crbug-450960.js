// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --stack-size=100

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-450960.js");
	}
}
"a".replace(/a/g, "");

var count = 0;
function test() {
   try {
     test();
   } catch(e) {
     if (count < 50) {
       count++;
       "b".replace(/(b)/g, new []);
     }
   }
}

try {
  test();
} catch (e) {
}
