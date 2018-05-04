// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Should not time out.  Running time 0.5s vs. 120s before the change.
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-482998.js");
	}
}
function collapse(flags) {
  var src = "(?:";
  for (var i = 128; i < 0x1000; i++) {
    src += String.fromCharCode(96 + i % 26) + String.fromCharCode(i) + "|";
  }
  src += "aa)";
  var collapsible = new RegExp(src, flags);
  var subject = "zzzzzzz" + String.fromCharCode(3000);
  for (var i = 0; i < 1000; i++) {
    subject += "xxxxxxx";
  }
  for (var i = 0; i < 2000; i++) {
    assertFalse(collapsible.test(subject));
  }
}

collapse("i");
collapse("");
