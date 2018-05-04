// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-358088.js");
	}
}
function f(a) {
  a[a.length] = 1;
}

function g(a, i, v) {
  a[i] = v;
}

f([]);    // f KeyedStoreIC goes to 1.GROW
o = {};
g(o);     // We've added property "undefined" to o

o = {};   // A transition on property "undefined" exists from {}
f(o);     // Store should go generic.
