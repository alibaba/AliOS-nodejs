// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-430846.js");
	}
}
function foo() { return 1; };
var o1 = {};
o1.foo = foo;

var json = '{"foo": {"x": 1}}';
var o2 = JSON.parse(json);
var o3 = JSON.parse(json);
assertTrue(%HaveSameMap(o2, o3));
