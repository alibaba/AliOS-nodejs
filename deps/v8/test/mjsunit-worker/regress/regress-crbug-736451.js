// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --expose-externalize-string --no-stress-opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-736451.js");
	}
}
!function() {
  const s0 = "external string turned into two byte";
  const s1 = s0.substring(1);
  try {
    externalizeString(s0, true);
  } catch(ex) {}

  s1.toLowerCase();
}();
