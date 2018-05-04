// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --stress-compaction

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-enum-prop-keys-cache-size.js");
	}
}
%SetAllocationTimeout(100000, 100000);

var x = {};
x.a = 1;
x.b = 2;
x = {};

var y = {};
y.a = 1;

%SetAllocationTimeout(100000, 0);

for (var z in y) { }
