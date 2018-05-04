// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-349853.js");
	}
}
var a = ["string"];
function funky(array) { return array[0] = 1; }
funky(a);

function crash() {
  var q = [0];
  // The failing ASSERT was only triggered when compiling for OSR.
  for (var i = 0; i < 100000; i++) {
    funky(q);
  }
  q[0] = 0;
  funky(q)
}

crash();
