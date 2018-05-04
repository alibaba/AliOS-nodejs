// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --stack-size=100

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-491062.js");
	}
}
function g() {}

var count = 0;
function f() {
  try {
    f();
  } catch(e) {
    print(e.stack);
  }
  if (count < 100) {
    count++;
    %DebugGetLoadedScripts();
  }
}
f();
g();
